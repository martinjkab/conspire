#pragma once
#include <rendering/utils/vk_types.h>

struct MeshBuffer {
  AllocatedBuffer vertexBuffer;
  AllocatedBuffer indexBuffer;
  VkDeviceAddress vertexBufferAddress;
  size_t indexCount;
  size_t vertexCount;
};