#pragma once 

#include <vulkan/vulkan.h>

namespace vkutil {
	void transitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);
}