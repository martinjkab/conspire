#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vulkan/vulkan.hpp>

#include "vk_types.h"

struct MeshBuffer {
  AllocatedBuffer vertexBuffer;
  AllocatedBuffer indexBuffer;
  VkDeviceAddress vertexBufferAddress;
};

class MeshStore {
 public:
  int add(std::vector<glm::vec4> vertices, std::vector<uint32_t> indices) {
    const size_t vertexBufferSize = vertices.size() * sizeof(glm::vec4);
    const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

    MeshBuffer buffer;

    buffer.vertexBuffer = create_buffer(
        vertexBufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);
  }

 private:
  std::unordered_map<int, MeshBuffer> meshes;
};