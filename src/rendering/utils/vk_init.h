#pragma once 

#include <vulkan/vulkan.h>

namespace vkinit {
    VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flags = 0);
    VkSemaphoreCreateInfo semaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);
    VkCommandBufferBeginInfo commandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
    VkImageSubresourceRange imageSubresourceRange(VkImageAspectFlags aspectMask);
    VkSemaphoreSubmitInfo semaphoreSubmitInfo(VkPipelineStageFlags2 stageMask, VkSemaphore semaphore);
    VkCommandBufferSubmitInfo commandBufferSubmitInfo(VkCommandBuffer cmd);
    VkSubmitInfo2 submitInfo(VkCommandBufferSubmitInfo* cmd, VkSemaphoreSubmitInfo* signalSemaphoreInfo, VkSemaphoreSubmitInfo* waitSemaphoreInfo);
    VkImageCreateInfo imageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);
    VkImageViewCreateInfo imageviewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);
}
