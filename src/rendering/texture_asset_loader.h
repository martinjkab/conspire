#pragma once

#include <ecs/asset_loader.h>
#include <ecs/asset_store.h>
#include <lodepng.h>
#include <rendering/mesh_buffer.h>
#include <rendering/utils/vk_utils.h>
#include <rendering/vertex.h>

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fmt/core.h>

#include "render_context.h"
#include "texture.h"
#include "uploader.h"
#include "utils/vk_images.h"
#include "utils/vk_init.h"
#include "utils/vk_utils.h"

template <>
AssetTraits<Texture>::CPUDataType
AssetLoader<Texture>::loadCPU(const std::string& path) const {
  std::vector<uint8_t> image;
  unsigned width, height;

  unsigned error = lodepng::decode(image, width, height, path);

  if (error) {
    fmt::print("Error loading sprite {}: {}\n", path,
               lodepng_error_text(error));
    return {};
  }

  fmt::print("Successfully loaded sprite: {} ({}x{}, RGBA)\n", path, width,
             height);
  return {image, width, height};
}

template <>
Texture
AssetLoader<Texture>::loadGPU(const AssetTraits<Texture>::CPUDataType& CPUData,
                              const RenderContext& context,
                              const Uploader& uploader) const {
  auto [data, width, height] = CPUData;
  AllocatedBuffer uploadbuffer =
      context.createBuffer(data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VMA_MEMORY_USAGE_CPU_TO_GPU);

  memcpy(uploadbuffer.info.pMappedData, data.data(), data.size());

  AllocatedImage image;

  image.imageFormat = VK_FORMAT_R8G8B8A8_UNORM;
  image.imageExtent = VkExtent3D{.width = width, .height = height, .depth = 1};

  VkImageCreateInfo info = vkinit::imageCreateInfo(
      image.imageFormat,
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
      image.imageExtent);

  VmaAllocationCreateInfo allocinfo = {};
  allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  allocinfo.requiredFlags =
      VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  VK_CHECK(vmaCreateImage(context.allocator, &info, &allocinfo, &image.image,
                          &image.allocation, nullptr));

  VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageViewCreateInfo view_info =
      vkinit::imageviewCreateInfo(image.imageFormat, image.image, aspectFlag);

  VK_CHECK(
      vkCreateImageView(context.device, &view_info, nullptr, &image.imageView));

  uploader.immediateSubmit(context, [&](VkCommandBuffer cmd) {
    vkutil::transitionImage(cmd, image.image, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkBufferImageCopy copyRegion = {};
    copyRegion.bufferOffset = 0;
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;

    copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copyRegion.imageSubresource.mipLevel = 0;
    copyRegion.imageSubresource.baseArrayLayer = 0;
    copyRegion.imageSubresource.layerCount = 1;
    copyRegion.imageExtent = image.imageExtent;

    // copy the buffer into the image
    vkCmdCopyBufferToImage(cmd, uploadbuffer.buffer, image.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &copyRegion);

    vkutil::transitionImage(cmd, image.image,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  });
  return {image};
}
