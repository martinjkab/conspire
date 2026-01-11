#pragma once

#include <ecs/asset_store.h>
#include <render_list.h>
#include <rendering/mesh_buffer.h>
#include <rendering/texture_buffer.h>
#include <rendering/utils/vk_descriptors.h>
#include <rendering/utils/vk_types.h>
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
  void mainLoop(const AssetStore& assetStore, const RenderList& renderList,
                const glm::mat4& projection);

  MeshBuffer uploadMesh(std::vector<Vertex> vertices,
                        std::vector<uint32_t> indices);
  MeshBuffer uploadGltf(const std::filesystem::path& path);
  TextureBuffer uploadTexture(const std::string& path);

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
  void draw(const AssetStore& assetStore, const RenderList& renderList,
            const glm::mat4& projection);
  void drawGeometry(VkCommandBuffer cmd, const AssetStore& assetStore,
                    const RenderList& renderList, const glm::mat4& projection);
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