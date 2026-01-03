#pragma once

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_MACOS_MVK
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#else
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#if (DEFINED ENV{DISPLAY})
#define GLFW_EXPOSE_NATIVE_X11
#endif()
#include <GLFW/glfw3native.h>
#endif

#include <vk_mem_alloc.h>

#include <cstdlib>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <filesystem>

#include "utils/vk_descriptors.h"
#include "utils/vk_types.h"
#include "ecs/asset_store.h"
#include "render_list.h"
#include <glm/glm.hpp>
#include "vertex.h"
#include "rendering/texture_buffer.h"
#include "rendering/mesh_buffer.h"

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
  void init();
  void mainLoop(const AssetStore& assetStore, const RenderList& renderList);

  MeshBuffer uploadMesh(std::vector<Vertex> vertices,
                        std::vector<uint32_t> indices);
  MeshBuffer uploadGltf(const std::filesystem::path& path);
  TextureBuffer uploadTexture(const std::string& path);
  void setKeyCallback(GLFWkeyfun callback);
  GLFWwindow* getWindow();

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
  VkQueue _presentQueue;
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

  void initWindow();
  void initInstance();
  void initSwapchain();
  void initCommands();
  void initSyncStructures();
  void draw(const AssetStore& assetStore, const RenderList& renderList);
  void drawGeometry(VkCommandBuffer cmd, const AssetStore& assetStore,
                    const RenderList& renderList);
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