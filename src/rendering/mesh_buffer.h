#pragma once
#include <rendering/utils/vk_types.h>
#include <rendering/vertex.h>

struct MeshBuffer {
  AllocatedBuffer vertexBuffer;
  AllocatedBuffer indexBuffer;
  VkDeviceAddress vertexBufferAddress;
  size_t indexCount;
  size_t vertexCount;
};

struct MeshData {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

template <> struct AssetTraits<MeshBuffer> {
  using CPUDataType = MeshData;
  static constexpr bool IsGpuAsset = true;
};