#pragma once
#include "utils/vk_types.h"

struct MeshBuffer {
  AllocatedBuffer vertexBuffer;
  AllocatedBuffer indexBuffer;
  VkDeviceAddress vertexBufferAddress;
};