#pragma once

#include "utils/vk_types.h"
#include "utils/vk_utils.h"
#include "vulkan/vulkan_core.h"

struct RenderContext {
  VkDevice device;
  VmaAllocator allocator;

  AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage,
                               VmaMemoryUsage memoryUsage) const {
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
    VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo,
                             &newBuffer.buffer, &newBuffer.allocation,
                             &newBuffer.info));

    return newBuffer;
  }
};