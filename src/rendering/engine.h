#pragma once

#include <VkBootstrap.h>
#include <core/platform.h>
#include <ecs/asset_store.h>
#include <fmt/base.h>
#include <lodepng.h>
#include <rendering/engine.h>
#include <rendering/mesh_buffer.h>
#include <rendering/render_list.h>
#include <rendering/texture.h>
#include <rendering/uniforms/global_uniform.h>
#include <rendering/utils/vk_descriptors.h>
#include <rendering/utils/vk_images.h>
#include <rendering/utils/vk_init.h>
#include <rendering/utils/vk_pipelines.h>
#include <rendering/utils/vk_types.h>
#include <rendering/utils/vk_utils.h>
#include <vertex.h>
#include <vk_mem_alloc.h>

#include <cstdlib>
#include <filesystem>
#include <functional>
#include <glm/glm.hpp>
#include <optional>
#include <string>
#include <vector>

struct GLFWwindow;

extern const char* APP_NAME;
extern const uint32_t WIDTH;
extern const uint32_t HEIGHT;

struct FrameData {
  VkCommandPool _commandPool;
  VkCommandBuffer _mainCommandBuffer;
  VkSemaphore _swapchainSemaphore;
  VkSemaphore _renderSemaphore;
  VkFence _renderFence;
  DescriptorAllocatorGrowable _frameDescriptors;
};

constexpr unsigned int FRAME_OVERLAP = 3;

class RenderEngine {
 public:
  void init(GLFWwindow* window);
  template <Material M>
  void mainLoop(const AssetStore& assetStore, const RenderList<M>& renderList,
                const glm::mat4& projection);

  MeshBuffer uploadMesh(std::vector<Vertex> vertices,
                        std::vector<uint32_t> indices);
  MeshBuffer uploadGltf(const std::filesystem::path& path);
  Texture uploadTexture(const std::string& path);

 private:
  FrameData _frames[FRAME_OVERLAP];
  int _frameNumber = 0;

  FrameData& getCurrentFrame() {
    return _frames[_frameNumber % FRAME_OVERLAP];
  };

  VkQueue _graphicsQueue;
  uint32_t _graphicsQueueFamily;
  GLFWwindow* _window;
  VkInstance _instance;
  VkDebugUtilsMessengerEXT _debugMessenger;
  VkPhysicalDevice _gpu = VK_NULL_HANDLE;
  VkDevice _device;
  VkSurfaceKHR _surface;
  VkSwapchainKHR _swapchain;
  std::vector<VkImage> _swapchainImages;
  std::vector<VkImageView> _swapchainImageViews;
  VkFormat _swapchainImageFormat;
  VkExtent2D _swapchainExtent;
  VkRenderPass _renderPass;
  VkPipelineLayout _pipelineLayout;
  VkPipeline _graphicsPipeline;
  std::vector<VkFramebuffer> _swapchainFramebuffers;
  VmaAllocator _allocator;
  AllocatedImage _drawImage;
  AllocatedImage _depthImage;
  VkExtent2D _drawExtent;
  DescriptorAllocator globalDescriptorAllocator;
  VkPipelineLayout _trianglePipelineLayout;
  VkPipeline _trianglePipeline;
  VkSampler _defaultSamplerNearest;

  VkDescriptorSetLayout _globalDescriptorLayout;
  VkDescriptorSetLayout _perObjectDescriptorLayout;

  VkCommandPool _immCommandPool;
  VkCommandBuffer _immCommandBuffer;
  VkFence _immFence;

  void initInstance();
  void initSwapchain();
  void initCommands();
  void initSyncStructures();
  template <Material M>
  void draw(const AssetStore& assetStore, const RenderList<M>& renderList,
            const glm::mat4& projection);
  template <Material M>
  void drawGeometry(VkCommandBuffer cmd, const AssetStore& assetStore,
                    const RenderList<M>& renderList,
                    const glm::mat4& projection);
  void initVulkan();
  void initDescriptors();
  void initPipelines();
  void initTrianglePipeline();
  std::tuple<std::vector<uint8_t>, unsigned, unsigned> loadSprite(
      std::string path);
  void cleanup();
  void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
  VkDeviceAddress getBufferDeviceAddress(VkBufferDeviceAddressInfo& info);
  AllocatedBuffer createBuffer(size_t allocSize, VkBufferUsageFlags usage,
                               VmaMemoryUsage memoryUsage);
};

template <Material M>
void RenderEngine::mainLoop(const AssetStore& assetStore,
                            const RenderList<M>& renderList,
                            const glm::mat4& projection) {
  draw(assetStore, renderList, projection);
  glfwPollEvents();
}

template <Material M>
void RenderEngine::draw(const AssetStore& assetStore,
                        const RenderList<M>& renderList,
                        const glm::mat4& projection) {
  VK_CHECK(vkWaitForFences(_device, 1, &getCurrentFrame()._renderFence, true,
                           1000000000));
  getCurrentFrame()._frameDescriptors.clearPools(_device);

  uint32_t swapchainImageIndex;
  VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000,
                                 getCurrentFrame()._swapchainSemaphore, nullptr,
                                 &swapchainImageIndex));

  VK_CHECK(vkResetFences(_device, 1, &getCurrentFrame()._renderFence));

  VK_CHECK(vkResetCommandBuffer(getCurrentFrame()._mainCommandBuffer, 0));

  VkCommandBuffer cmd = getCurrentFrame()._mainCommandBuffer;

  _drawExtent.width = _drawImage.imageExtent.width;
  _drawExtent.height = _drawImage.imageExtent.height;

  VkCommandBufferBeginInfo cmdBeginInfo = vkinit::commandBufferBeginInfo(
      VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

  vkutil::transitionImage(cmd, _drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

  vkutil::transitionImage(cmd, _depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

  drawGeometry(cmd, assetStore, renderList, projection);

  vkutil::transitionImage(cmd, _drawImage.image,
                          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                          VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
  vkutil::transitionImage(cmd, _swapchainImages[swapchainImageIndex],
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  vkutil::copyImageToImage(cmd, _drawImage.image,
                           _swapchainImages[swapchainImageIndex], _drawExtent,
                           _swapchainExtent);
  vkutil::transitionImage(cmd, _swapchainImages[swapchainImageIndex],
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

  VK_CHECK(vkEndCommandBuffer(cmd));

  VkCommandBufferSubmitInfo cmdinfo = vkinit::commandBufferSubmitInfo(cmd);

  VkSemaphoreSubmitInfo waitInfo = vkinit::semaphoreSubmitInfo(
      VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
      getCurrentFrame()._swapchainSemaphore);
  VkSemaphoreSubmitInfo signalInfo = vkinit::semaphoreSubmitInfo(
      VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, getCurrentFrame()._renderSemaphore);

  VkSubmitInfo2 submit = vkinit::submitInfo(&cmdinfo, &signalInfo, &waitInfo);

  VK_CHECK(vkQueueSubmit2(_graphicsQueue, 1, &submit,
                          getCurrentFrame()._renderFence));

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  presentInfo.pSwapchains = &_swapchain;
  presentInfo.swapchainCount = 1;

  presentInfo.pWaitSemaphores = &getCurrentFrame()._renderSemaphore;
  presentInfo.waitSemaphoreCount = 1;

  presentInfo.pImageIndices = &swapchainImageIndex;

  VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

  _frameNumber++;
}

template <Material M>
void RenderEngine::drawGeometry(VkCommandBuffer cmd,
                                const AssetStore& assetStore,
                                const RenderList<M>& renderList,
                                const glm::mat4& projection) {
  VkClearColorValue clearColorValue{0.0};
  VkClearValue clearValue{.color = clearColorValue};
  VkRenderingAttachmentInfo colorAttachment =
      vkinit::attachmentInfo(_drawImage.imageView, &clearValue,
                             VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
  VkClearColorValue depthClearColorValue{1.0};
  VkClearValue depthClearValue{.color = depthClearColorValue};
  VkRenderingAttachmentInfo depthAttachment =
      vkinit::attachmentInfo(_depthImage.imageView, &depthClearValue,
                             VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL);

  VkRenderingInfo renderInfo =
      vkinit::renderingInfo(_drawExtent, &colorAttachment, &depthAttachment);
  vkCmdBeginRendering(cmd, &renderInfo);

  vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipeline);

  VkViewport viewport = {};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = _drawExtent.width;
  viewport.height = _drawExtent.height;
  viewport.minDepth = 0.f;
  viewport.maxDepth = 1.f;

  vkCmdSetViewport(cmd, 0, 1, &viewport);

  VkRect2D scissor = {};
  scissor.offset.x = 0;
  scissor.offset.y = 0;
  scissor.extent.width = _drawExtent.width;
  scissor.extent.height = _drawExtent.height;

  vkCmdSetScissor(cmd, 0, 1, &scissor);

  {
    auto globalLayouts = std::vector{_globalDescriptorLayout};
    std::vector<VkDescriptorSet> globalSet =
        getCurrentFrame()._frameDescriptors.allocate(_device, globalLayouts);

    DescriptorWriter writer;
    AllocatedBuffer uploadbuffer = createBuffer(
        sizeof(GlobalUniform), VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT,
        VMA_MEMORY_USAGE_CPU_TO_GPU);
    GlobalUniform uniform{projection};
    memcpy(uploadbuffer.info.pMappedData, &uniform, sizeof(GlobalUniform));
    writer.writeBuffer(0, uploadbuffer.buffer, sizeof(GlobalUniform), 0,
                       VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    writer.updateSet(_device, globalSet.at(0));

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            _trianglePipelineLayout, 0, globalSet.size(),
                            globalSet.data(), 0, nullptr);
  }

  for (auto item : renderList.items) {
    auto imageSetLayouts = std::vector{_perObjectDescriptorLayout};
    std::vector<VkDescriptorSet> imageSet =
        getCurrentFrame()._frameDescriptors.allocate(_device, imageSetLayouts);
    assetStore[item.material].value().uploadUniforms(
        {assetStore, _device, _defaultSamplerNearest});
    {
      DescriptorWriter writer;
      AllocatedBuffer uploadbuffer =
          createBuffer(sizeof(glm::mat4), VK_BUFFER_USAGE_2_UNIFORM_BUFFER_BIT,
                       VMA_MEMORY_USAGE_CPU_TO_GPU);
      // auto scaled = glm::scale(
      //     item.model, glm::vec3(textureData.image.imageExtent.width,
      //                           textureData.image.imageExtent.height, 0));
      memcpy(uploadbuffer.info.pMappedData, &(item.model), sizeof(glm::mat4));
      writer.writeBuffer(1, uploadbuffer.buffer, sizeof(glm::mat4), 0,
                         VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
      writer.updateSet(_device, imageSet.at(0));
    }
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            _trianglePipelineLayout, 1, imageSet.size(),
                            imageSet.data(), 0, nullptr);

    auto bufferData = assetStore[item.mesh].value();
    vkCmdPushConstants(cmd, _trianglePipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
                       0, sizeof(VkDeviceAddress),
                       &(bufferData.vertexBufferAddress));
    vkCmdBindIndexBuffer(cmd, bufferData.indexBuffer.buffer, 0,
                         VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(cmd, bufferData.indexCount, 1, 0, 0, 0);
  }

  vkCmdEndRendering(cmd);
}