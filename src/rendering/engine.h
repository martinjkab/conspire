#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <optional>
#include <vector>
#include <cstdlib>

#include <vk_mem_alloc.h>
#include "utils/vk_types.h"
#include "utils/vk_descriptors.h"

extern const char *APP_NAME;
extern const uint32_t WIDTH;
extern const uint32_t HEIGHT;

struct FrameData
{
    VkCommandPool _commandPool;
    VkCommandBuffer _mainCommandBuffer;
    VkSemaphore _swapchainSemaphore;
    VkSemaphore _renderSemaphore;
    VkFence _renderFence;
};

constexpr unsigned int FRAME_OVERLAP = 2;

class RenderEngine
{
public:
    void init();
    void mainLoop();

private:
    FrameData _frames[FRAME_OVERLAP];
    int _frameNumber = 0;

    FrameData &getCurrentFrame() { return _frames[_frameNumber % FRAME_OVERLAP]; };

    VkQueue _graphicsQueue;
    uint32_t _graphicsQueueFamily;
    GLFWwindow *_window;
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
    VkDescriptorSet _drawImageDescriptors;
    VkDescriptorSetLayout _drawImageDescriptorLayout;
    VkPipeline _gradientPipeline;
    VkPipelineLayout _gradientPipelineLayout;

    void initWindow();
    void initInstance();
    void initSwapchain();
    void initCommands();
    void initSyncStructures();
    void draw();
    void drawBackground(VkCommandBuffer cmd) const;
    void initVulkan();
    void initDescriptors();
    void initPipelines();
    void initBackgroundPipelines();
    void cleanup();
};