#pragma once

#include "render_context.h"
#include "utils/vk_init.h"
#include "utils/vk_utils.h"
#include "vulkan/vulkan_core.h"
#include <functional>

class Uploader {
public:
  Uploader() = default;
  Uploader(VkCommandBuffer commandBuffer, VkFence fence, VkQueue queue)
      : _immCommandBuffer(commandBuffer), _immFence(fence),
        _graphicsQueue(queue) {}

  void
  immediateSubmit(const RenderContext& context,
                  std::function<void(VkCommandBuffer cmd)>&& function) const {
    VK_CHECK(vkResetFences(context.device, 1, &_immFence));
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

    VK_CHECK(vkWaitForFences(context.device, 1, &_immFence, true, UINT64_MAX));
  }

private:
  VkCommandBuffer _immCommandBuffer;
  VkFence _immFence;
  VkQueue _graphicsQueue;
};