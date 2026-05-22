#pragma once

#include <ecs/asset_loader.h>
#include <ecs/asset_store.h>
#include <rendering/mesh_buffer.h>
#include <rendering/utils/vk_utils.h>
#include <rendering/vertex.h>

#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <filesystem>
#include <limits>

#include "render_context.h"
#include "uploader.h"

template <>
AssetTraits<MeshBuffer>::CPUDataType
AssetLoader<MeshBuffer>::loadCPU(const std::string& path) const {
  fastgltf::Parser parser;
  auto data = fastgltf::GltfDataBuffer::FromPath(path);

  if (data.error() != fastgltf::Error::None) {
    throw data.error();
  }

  auto assetResult = parser.loadGltfBinary(
      data.get(), std::filesystem::path(path).parent_path(),
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

  return {vertices, indices};
}

template <>
MeshBuffer AssetLoader<MeshBuffer>::loadGPU(
    const AssetTraits<MeshBuffer>::CPUDataType& CPUData,
    const RenderContext& context, const Uploader& uploader) const {
  const auto& [vertices, indices] = CPUData;
  const size_t vertexBufferSize = vertices.size() * sizeof(Vertex);
  const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

  MeshBuffer buffer;
  buffer.indexCount = indices.size();
  buffer.vertexCount = vertices.size();
  buffer.bounds = MeshBounds{.min = glm::vec3{0.0f}, .max = glm::vec3{0.0f}};
  if (!vertices.empty()) {
    buffer.bounds = MeshBounds{
        .min = glm::vec3{std::numeric_limits<float>::max()},
        .max = glm::vec3{std::numeric_limits<float>::lowest()}};

    for (const auto& vertex : vertices) {
      const auto position = glm::vec3{vertex.position};
      buffer.bounds.min = glm::min(buffer.bounds.min, position);
      buffer.bounds.max = glm::max(buffer.bounds.max, position);
    }
  }

  buffer.vertexBuffer = context.createBuffer(
      vertexBufferSize,
      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);

  VkBufferDeviceAddressInfo deviceAddressInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
      .buffer = buffer.vertexBuffer.buffer};
  buffer.vertexBufferAddress =
      vkGetBufferDeviceAddress(context.device, &deviceAddressInfo);

  buffer.indexBuffer = context.createBuffer(
      indexBufferSize,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);

  AllocatedBuffer staging = context.createBuffer(
      vertexBufferSize + indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      VMA_MEMORY_USAGE_CPU_ONLY);

  void* data = staging.info.pMappedData;
  memcpy(data, vertices.data(), vertexBufferSize);
  memcpy((char*)data + vertexBufferSize, indices.data(), indexBufferSize);

  uploader.immediateSubmit(context, [&](VkCommandBuffer cmd) {
    VkBufferCopy vertexCopy{
        .srcOffset = 0, .dstOffset = 0, .size = vertexBufferSize};
    vkCmdCopyBuffer(cmd, staging.buffer, buffer.vertexBuffer.buffer, 1,
                    &vertexCopy);

    VkBufferCopy indexCopy{
        .srcOffset = vertexBufferSize, .dstOffset = 0, .size = indexBufferSize};
    vkCmdCopyBuffer(cmd, staging.buffer, buffer.indexBuffer.buffer, 1,
                    &indexCopy);
  });

  vkDestroyBuffer(context.device, staging.buffer, nullptr);

  return buffer;
}
