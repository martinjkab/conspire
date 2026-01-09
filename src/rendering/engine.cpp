#define VMA_IMPLEMENTATION

#include <VkBootstrap.h>
#include <core/platform.h>
#include <ecs/asset_store.h>
#include <fmt/base.h>
#include <lodepng.h>
#include <rendering/engine.h>
#include <rendering/render_list.h>
#include <rendering/uniforms/global_uniform.h>
#include <rendering/utils/vk_images.h>
#include <rendering/utils/vk_init.h>
#include <rendering/utils/vk_pipelines.h>
#include <rendering/utils/vk_utils.h>
#include <vk_mem_alloc.h>

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

const char* APP_NAME = "Conspire";
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

void RenderEngine::init(GLFWwindow* window) {
  _window = window;
  initVulkan();
}

void RenderEngine::initInstance() {
  vkb::InstanceBuilder builder;

  auto inst_ret = builder.set_app_name(APP_NAME)
                      .request_validation_layers(true)
                      .use_default_debug_messenger()
                      .require_api_version(1, 4, 3)
                      .build();

  vkb::Instance vkbInst = inst_ret.value();

  _instance = vkbInst.instance;
  _debugMessenger = vkbInst.debug_messenger;

  glfwCreateWindowSurface(_instance, _window, nullptr, &_surface);

  // vulkan 1.4 features
  // VkPhysicalDeviceVulkan13Features features14{ .sType =
  // VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES };
  // features14.dynamicRendering = true;
  // features14.synchronization2 = true;

  // vulkan 1.3 features
  VkPhysicalDeviceVulkan13Features features13{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
  features13.dynamicRendering = true;
  features13.synchronization2 = true;

  // vulkan 1.2 features
  VkPhysicalDeviceVulkan12Features features12{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
  features12.bufferDeviceAddress = true;
  features12.descriptorIndexing = true;

  vkb::PhysicalDeviceSelector selector{vkbInst};
  vkb::PhysicalDevice physicalDevice =
      selector
          .set_minimum_version(1, 3)
          //.set_required_features_14(features14)
          .set_required_features_13(features13)
          .set_required_features_12(features12)
          .set_surface(_surface)
          .select()
          .value();

  vkb::DeviceBuilder deviceBuilder{physicalDevice};

  vkb::Device vkbDevice = deviceBuilder.build().value();

  _device = vkbDevice.device;
  _gpu = physicalDevice.physical_device;

  _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
  _graphicsQueueFamily =
      vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void RenderEngine::initSwapchain() {
  vkb::SwapchainBuilder swapchainBuilder{_gpu, _device, _surface};

  _swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

  vkb::Swapchain vkbSwapchain =
      swapchainBuilder
          .set_desired_format(VkSurfaceFormatKHR{
              .format = _swapchainImageFormat,
              .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR})
          .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
          .set_desired_extent(WIDTH, HEIGHT)
          .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
          .build()
          .value();

  _swapchainExtent = vkbSwapchain.extent;
  _swapchain = vkbSwapchain.swapchain;
  _swapchainImages = vkbSwapchain.get_images().value();
  _swapchainImageViews = vkbSwapchain.get_image_views().value();

  VkExtent3D drawImageExtent = {WIDTH, HEIGHT, 1};

  _drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
  _drawImage.imageExtent = drawImageExtent;

  _depthImage.imageFormat = VK_FORMAT_D32_SFLOAT;
  _depthImage.imageExtent = drawImageExtent;

  VkImageUsageFlags depthImageUsages =
      VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  VkImageCreateInfo dimg_info = vkinit::imageCreateInfo(
      _depthImage.imageFormat, depthImageUsages, drawImageExtent);

  VkImageUsageFlags drawImageUsages{};
  drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
  drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  VkImageCreateInfo rimg_info = vkinit::imageCreateInfo(
      _drawImage.imageFormat, drawImageUsages, drawImageExtent);

  VmaAllocationCreateInfo rimg_allocinfo = {};
  rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
  rimg_allocinfo.requiredFlags =
      VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  vmaCreateImage(_allocator, &rimg_info, &rimg_allocinfo, &_drawImage.image,
                 &_drawImage.allocation, nullptr);

  vmaCreateImage(_allocator, &dimg_info, &rimg_allocinfo, &_depthImage.image,
                 &_depthImage.allocation, nullptr);

  VkImageViewCreateInfo rview_info = vkinit::imageviewCreateInfo(
      _drawImage.imageFormat, _drawImage.image, VK_IMAGE_ASPECT_COLOR_BIT);

  VK_CHECK(
      vkCreateImageView(_device, &rview_info, nullptr, &_drawImage.imageView));

  VkImageViewCreateInfo dview_info = vkinit::imageviewCreateInfo(
      _depthImage.imageFormat, _depthImage.image, VK_IMAGE_ASPECT_DEPTH_BIT);

  VK_CHECK(
      vkCreateImageView(_device, &dview_info, nullptr, &_depthImage.imageView));
}

void RenderEngine::initCommands() {
  VkCommandPoolCreateInfo commandPoolInfo = {};
  commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  commandPoolInfo.pNext = nullptr;
  commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  commandPoolInfo.queueFamilyIndex = _graphicsQueueFamily;

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr,
                                 &_frames[i]._commandPool));

    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;
    cmdAllocInfo.commandPool = _frames[i]._commandPool;
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo,
                                      &_frames[i]._mainCommandBuffer));
  }

  VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr,
                               &_immCommandPool));

  VkCommandBufferAllocateInfo cmdAllocInfo =
      vkinit::commandBufferAllocateInfo(_immCommandPool, 1);

  VK_CHECK(
      vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_immCommandBuffer));
}

void RenderEngine::initSyncStructures() {
  VkFenceCreateInfo fenceCreateInfo =
      vkinit::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
  VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphoreCreateInfo();

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr,
                           &_frames[i]._renderFence));

    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
                               &_frames[i]._swapchainSemaphore));
    VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr,
                               &_frames[i]._renderSemaphore));
  }

  VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_immFence));
}

void RenderEngine::draw(const AssetStore& assetStore,
                        const RenderList& renderList,
                        const glm::mat4& projection) {
  VK_CHECK(vkWaitForFences(_device, 1, &getCurrentFrame()._renderFence, true,
                           1000000000));
  getCurrentFrame()._frameDescriptors.clearPools(_device);

  uint32_t swapchainImageIndex;
  VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000,
                                 getCurrentFrame()._swapchainSemaphore, nullptr,
                                 &swapchainImageIndex));

  VK_CHECK(vkResetFences(_device, 1, &getCurrentFrame()._renderFence));

  VK_CHECK(vkResetCommandBuffer(getCurrentFrame()._mainCommandBuffer, 0));

  VkCommandBuffer cmd = getCurrentFrame()._mainCommandBuffer;

  _drawExtent.width = _drawImage.imageExtent.width;
  _drawExtent.height = _drawImage.imageExtent.height;

  VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

  vkutil::transitionImage(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  vkutil::transitionImage(cmd, _depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

  drawGeometry(cmd, assetStore, renderList, projection);

  vkutil::transitionImage(cmd, _drawImage.image,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  vkutil::transitionImage(cmd, _swapchainImages[swapchainImageIndex],
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  vkutil::copyImageToImage(cmd, _drawImage.image,
                           _swapchainImages[swapchainImageIndex], _drawExtent,
                           _swapchainExtent);
  vkutil::transitionImage(cmd, _swapchainImages[swapchainImageIndex],
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  VK_CHECK(vkEndCommandBuffer(cmd));

  VkCommandBufferSubmitInfo cmdinfo = vkinit::commandBufferSubmitInfo(cmd);

  VkSemaphoreSubmitInfo waitInfo = vkinit::semaphoreSubmitInfo(
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
      getCurrentFrame()._swapchainSemaphore);
  VkSemaphoreSubmitInfo signalInfo = vkinit::semaphoreSubmitInfo(
      VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame()._renderSemaphore);

  VkSubmitInfo2 submit = vkinit::submitInfo(&cmdinfo, &signalInfo, &waitInfo);

  VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit,
                          getCurrentFrame()._renderFence));

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  presentInfo.pSwapchains = &_swapchain;
  presentInfo.swapchainCount = 1;

  presentInfo.pWaitSemaphores = &getCurrentFrame()._renderSemaphore;
  presentInfo.waitSemaphoreCount = 1;

  presentInfo.pImageIndices = &swapchainImageIndex;

  VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

  _frameNumber++;
}

void RenderEngine::drawGeometry(VkCommandBuffer cmd,
                                const AssetStore& assetStore,
                                const RenderList& renderList,
                                const glm::mat4& projection) {
  VkClearColorValue clearColorValue{0.0};
  VkClearValue clearValue{.color = clearColorValue};
  VkRenderingAttachmentInfo colorAttachment =
      vkinit::attachmentInfo(_drawImage.imageView, &clearValue,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  VkClearColorValue depthClearColorValue{1.0};
  VkClearValue depthClearValue{.color = depthClearColorValue};
  VkRenderingAttachmentInfo depthAttachment =
      vkinit::attachmentInfo(_depthImage.imageView, &depthClearValue,
                             VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

  VkRenderingInfo renderInfo =
      vkinit::renderingInfo(_drawExtent, &colorAttachment, &depthAttachment);
  vkCmdBeginRendering(cmd, &renderInfo);

  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipeline);

  VkViewport viewport = {};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = _drawExtent.width;
  viewport.height = _drawExtent.height;
  viewport.minDepth = 0.f;
  viewport.maxDepth = 1.f;

  vkCmdSetViewport(cmd, 0, 1, &viewport);

  VkRect2D scissor = {};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = _drawExtent.width;
  scissor.extent.height = _drawExtent.height;

  vkCmdSetScissor(cmd, 0, 1, &scissor);

  {
    auto globalLayouts = std::vector{_globalDescriptorLayout};
    std::vector<VkDescriptorSet> globalSet =
        getCurrentFrame()._frameDescriptors.allocate(_device, globalLayouts);

    DescriptorWriter writer;
    AllocatedBuffer uploadbuffer = createBuffer(
        sizeof(GlobalUniform), VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU);
    GlobalUniform uniform{projection};
    memcpy(uploadbuffer.info.pMappedData, &uniform, sizeof(GlobalUniform));
    writer.writeBuffer(0, uploadbuffer.buffer, sizeof(GlobalUniform), 0,
                       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    writer.updateSet(_device, globalSet.at(0));

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            _trianglePipelineLayout, 0, globalSet.size(),
                            globalSet.data(), 0, nullptr);
  }

  for (auto item : renderList.items) {
    auto imageSetLayouts = std::vector{_perObjectDescriptorLayout};
    std::vector<VkDescriptorSet> imageSet =
        getCurrentFrame()._frameDescriptors.allocate(_device, imageSetLayouts);
    auto textureData = assetStore[item.textureHandle];
    {
      DescriptorWriter writer;
      writer.writeImage(0, textureData.image.imageView, _defaultSamplerNearest,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
      writer.updateSet(_device, imageSet.at(0));
    }
    {
      DescriptorWriter writer;
      AllocatedBuffer uploadbuffer =
          createBuffer(sizeof(glm::mat4), VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT,
                       VMA_MEMORY_USAGE_CPU_TO_GPU);
      // auto scaled = glm::scale(
      //     item.model, glm::vec3(textureData.image.imageExtent.width,
      //                           textureData.image.imageExtent.height, 0));
      memcpy(uploadbuffer.info.pMappedData, &(item.model), sizeof(glm::mat4));
      writer.writeBuffer(1, uploadbuffer.buffer, sizeof(glm::mat4), 0,
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
      writer.updateSet(_device, imageSet.at(0));
    }
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            _trianglePipelineLayout, 1, imageSet.size(),
                            imageSet.data(), 0, nullptr);

    auto bufferData = assetStore[item.meshHandle];
    vkCmdPushConstants(cmd, _trianglePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
                       0, sizeof(VkDeviceAddress),
                       &(bufferData.vertexBufferAddress));
    vkCmdBindIndexBuffer(cmd, bufferData.indexBuffer.buffer, 0,
                         VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd, bufferData.indexCount, 1, 0, 0, 0);
  }

  vkCmdEndRendering(cmd);
}

MeshBuffer RenderEngine::uploadMesh(std::vector<Vertex> vertices,
                                    std::vector<uint32_t> indices) {
  const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
  const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

  MeshBuffer buffer;

  buffer.indexCount = indices.size();
  buffer.vertexCount = vertices.size();

  buffer.vertexBuffer = createBuffer(
      vertexBufferSize,
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);

  VkBufferDeviceAddressInfo deviceAddressInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .buffer = buffer.vertexBuffer.buffer};
  buffer.vertexBufferAddress = getBufferDeviceAddress(deviceAddressInfo);

  buffer.indexBuffer = createBuffer(
      indexBufferSize,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);

  AllocatedBuffer staging =
      createBuffer(vertexBufferSize + indexBufferSize,
                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

  void* data = staging.allocation->GetMappedData();

  memcpy(data, vertices.data(), vertexBufferSize);
  memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

  immediateSubmit([&](VkCommandBuffer cmd) {
    VkBufferCopy vertexCopy{0};
    vertexCopy.dstOffset = 0;
    vertexCopy.srcOffset = 0;
    vertexCopy.size = vertexBufferSize;

    vkCmdCopyBuffer(cmd, staging.buffer, buffer.vertexBuffer.buffer, 1,
                    &vertexCopy);

    VkBufferCopy indexCopy{0};
    indexCopy.dstOffset = 0;
    indexCopy.srcOffset = vertexBufferSize;
    indexCopy.size = indexBufferSize;

    vkCmdCopyBuffer(cmd, staging.buffer, buffer.indexBuffer.buffer, 1,
                    &indexCopy);
  });

  vkDestroyBuffer(_device, staging.buffer, nullptr);

  return buffer;
}

MeshBuffer RenderEngine::uploadGltf(const std::filesystem::path& path) {
  fastgltf::Parser parser;
  auto data = fastgltf::GltfDataBuffer::FromPath(path);

  if (data.error() != fastgltf::Error::None) {
    throw data.error();
  }

  auto assetResult =
      parser.loadGltfBinary(data.get(), path.parent_path(),
                            fastgltf::Options::LoadGLBBuffers |
                                fastgltf::Options::LoadExternalBuffers);
  if (assetResult.error() != fastgltf::Error::None) {
    throw assetResult.error();
  }
  const auto& asset = assetResult.get();

  const auto& primitive = asset.meshes.at(0).primitives.at(0);

  auto& posAccessor =
      asset.accessors[primitive.findAttribute("POSITION")->accessorIndex];
  auto& uvAccessor =
      asset.accessors[primitive.findAttribute("TEXCOORD_0")->accessorIndex];
  auto& indexAccessor = asset.accessors[primitive.indicesAccessor.value()];

  std::vector<Vertex> vertices(posAccessor.count);
  std::vector<uint32_t> indices(indexAccessor.count);

  fastgltf::iterateAccessorWithIndex<std::uint32_t>(
      asset, indexAccessor,
      [&](std::uint32_t index, size_t idx) { indices[idx] = index; });

  fastgltf::iterateAccessorWithIndex<glm::vec3>(
      asset, posAccessor, [&](glm::vec3 p, size_t idx) {
        vertices[idx].position = glm::vec4{p, 1.};
      });

  fastgltf::iterateAccessorWithIndex<glm::vec2>(
      asset, uvAccessor,
      [&](glm::vec2 uv, size_t idx) { vertices[idx].tex = uv; });

  return uploadMesh(vertices, indices);
}

TextureBuffer RenderEngine::uploadTexture(const std::string& path) {
  auto [data, width, height] = loadSprite(path);
  AllocatedBuffer uploadbuffer =
      createBuffer(data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
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

  VK_CHECK(vmaCreateImage(_allocator, &info, &allocinfo, &image.image,
                          &image.allocation, nullptr));

  VkImageAspectFlags aspectFlag = VK_IMAGE_ASPECT_COLOR_BIT;

  VkImageViewCreateInfo view_info =
      vkinit::imageviewCreateInfo(image.imageFormat, image.image, aspectFlag);

  VK_CHECK(vkCreateImageView(_device, &view_info, nullptr, &image.imageView));

  immediateSubmit([&](VkCommandBuffer cmd) {
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

AllocatedBuffer RenderEngine::createBuffer(size_t allocSize,
                                           VkBufferUsageFlags usage,
                                           VmaMemoryUsage memoryUsage) {
  VkBufferCreateInfo bufferInfo = {.sType =
                                       VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferInfo.pNext = nullptr;
  bufferInfo.size = allocSize;

  bufferInfo.usage = usage;

  VmaAllocationCreateInfo vmaallocInfo = {};
  vmaallocInfo.usage = memoryUsage;
  vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
  AllocatedBuffer newBuffer;

  // allocate the buffer
  VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &vmaallocInfo,
                           &newBuffer.buffer, &newBuffer.allocation,
                           &newBuffer.info));

  return newBuffer;
}

VkDeviceAddress RenderEngine::getBufferDeviceAddress(
    VkBufferDeviceAddressInfo& info) {
  return vkGetBufferDeviceAddress(_device, &info);
}

void RenderEngine::initVulkan() {
  initInstance();

  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = _gpu;
  allocatorInfo.device = _device;
  allocatorInfo.instance = _instance;
  allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
  vmaCreateAllocator(&allocatorInfo, &_allocator);

  initSwapchain();
  initCommands();
  initSyncStructures();
  initDescriptors();
  initPipelines();
}

void RenderEngine::initDescriptors() {
  std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {
      {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};

  globalDescriptorAllocator.initPool(_device, 10, sizes);

  {
    DescriptorLayoutBuilder builder;
    builder.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    builder.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    _perObjectDescriptorLayout = builder.build(
        _device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
  }

  {
    DescriptorLayoutBuilder builder;
    builder.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    _globalDescriptorLayout = builder.build(
        _device, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
  }

  for (int i = 0; i < FRAME_OVERLAP; i++) {
    std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = {
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4},
    };

    _frames[i]._frameDescriptors = DescriptorAllocatorGrowable{};
    _frames[i]._frameDescriptors.init(_device, 1000, frame_sizes);
  }
}

void RenderEngine::initPipelines() { initTrianglePipeline(); }

void RenderEngine::initTrianglePipeline() {
  VkShaderModule triangleFragShader;
  static const unsigned char fragmentData[] = {
#embed "shaders/sprite.frag.spv"
  };
  if (!vkutil::createShaderModule(fragmentData, sizeof(fragmentData), _device,
                                  &triangleFragShader)) {
    fmt::print("Error when building the triangle fragment shader module");
  } else {
    fmt::print("Triangle fragment shader succesfully loaded");
  }

  VkShaderModule triangleVertexShader;
  static const unsigned char vertexData[] = {
#embed "shaders/sprite.vert.spv"
  };

  if (!vkutil::createShaderModule(vertexData, sizeof(vertexData), _device,
                                  &triangleVertexShader)) {
    fmt::print("Error when building the triangle vertex shader module");
  } else {
    fmt::print("Triangle vertex shader succesfully loaded");
  }

  VkPushConstantRange bufferRange{};
  bufferRange.offset = 0;
  bufferRange.size = sizeof(VkDeviceAddress);
  bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo =
      vkinit::pipelineLayoutCreateInfo();

  auto layouts =
      std::array{_globalDescriptorLayout, _perObjectDescriptorLayout};
  pipelineLayoutInfo.pSetLayouts = layouts.data();
  pipelineLayoutInfo.setLayoutCount = layouts.size();
  pipelineLayoutInfo.pPushConstantRanges = &bufferRange;
  pipelineLayoutInfo.pushConstantRangeCount = 1;

  VkSamplerCreateInfo sampler = {.sType =
                                     VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

  sampler.magFilter = VK_FILTER_NEAREST;
  sampler.minFilter = VK_FILTER_NEAREST;

  vkCreateSampler(_device, &sampler, nullptr, &_defaultSamplerNearest);

  VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr,
                                  &_trianglePipelineLayout));

  PipelineBuilder pipelineBuilder;

  pipelineBuilder._pipelineLayout = _trianglePipelineLayout;
  pipelineBuilder.setShaders(triangleVertexShader, triangleFragShader);
  pipelineBuilder.setInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  pipelineBuilder.setPolygonMode(VK_POLYGON_MODE_FILL);
  pipelineBuilder.setCullMode(VK_CULL_MODE_BACK_BIT,
                              VK_FRONT_FACE_COUNTER_CLOCKWISE);
  pipelineBuilder.setMultisamplingNone();
  pipelineBuilder.disableBlending();
  pipelineBuilder.enableDepthtest();
  // pipelineBuilder.disableDepthtest();

  pipelineBuilder.setColorAttachmentFormat(_drawImage.imageFormat);
  pipelineBuilder.setDepthFormat(VK_FORMAT_D32_SFLOAT);

  _trianglePipeline = pipelineBuilder.buildPipeline(_device);

  vkDestroyShaderModule(_device, triangleFragShader, nullptr);
  vkDestroyShaderModule(_device, triangleVertexShader, nullptr);
}

std::tuple<std::vector<uint8_t>, unsigned, unsigned> RenderEngine::loadSprite(
    std::string path) {
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

void RenderEngine::mainLoop(const AssetStore& assetStore,
                            const RenderList& renderList,
                            const glm::mat4& projection) {
  draw(assetStore, renderList, projection);
  glfwPollEvents();
}

void RenderEngine::cleanup() {
  for (auto framebuffer : _swapchainFramebuffers) {
    vkDestroyFramebuffer(_device, framebuffer, nullptr);
  }
  vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
  vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
  vkDestroyRenderPass(_device, _renderPass, nullptr);
  for (auto imageView : _swapchainImageViews) {
    vkDestroyImageView(_device, imageView, nullptr);
  }

  vkDestroySwapchainKHR(_device, _swapchain, nullptr);
  vkDestroyDevice(_device, nullptr);

  vkDestroySurfaceKHR(_instance, _surface, nullptr);
  vkDestroyInstance(_instance, nullptr);

  glfwDestroyWindow(_window);

  glfwTerminate();
}

void RenderEngine::immediateSubmit(
    std::function<void(VkCommandBuffer cmd)>&& function) {
  VK_CHECK(vkResetFences(_device, 1, &_immFence));
  VK_CHECK(vkResetCommandBuffer(_immCommandBuffer, 0));

  VkCommandBuffer cmd = _immCommandBuffer;

  VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

  VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

  function(cmd);

  VK_CHECK(vkEndCommandBuffer(cmd));

  VkCommandBufferSubmitInfo cmdinfo = vkinit::commandBufferSubmitInfo(cmd);
  VkSubmitInfo2 submit = vkinit::submitInfo(&cmdinfo, nullptr, nullptr);

  VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit, _immFence));

  VK_CHECK(vkWaitForFences(_device, 1, &_immFence, true, 9999999999));
}