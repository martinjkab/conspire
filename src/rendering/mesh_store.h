#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include <memory>

#include "utils/vk_types.h"
#include "engine.h"

struct MeshBuffer {
  AllocatedBuffer vertexBuffer;
  AllocatedBuffer indexBuffer;
  VkDeviceAddress vertexBufferAddress;
};

class MeshStore {
 public:
  MeshStore(std::shared_ptr<RenderEngine> engine) : engine{engine} {};

  int add(std::vector<glm::vec4> vertices, std::vector<uint32_t> indices) {
    const size_t vertexBufferSize = vertices.size() * sizeof(glm::vec4);
    const size_t indexBufferSize = indices.size() * sizeof(uint32_t);

    MeshBuffer buffer;

    buffer.vertexBuffer = engine->createBuffer(
        vertexBufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo deviceAddressInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .buffer = buffer.vertexBuffer.buffer};
    buffer.vertexBufferAddress =
        engine->getBufferDeviceAddress(deviceAddressInfo);

    buffer.vertexBuffer = engine->createBuffer(
        indexBufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY);

    meshes[counter++] = buffer;
  }

 private:
  std::shared_ptr<RenderEngine> engine;
  std::unordered_map<int, MeshBuffer> meshes;
  volatile int counter = 0;
};