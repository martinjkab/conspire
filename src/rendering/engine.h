#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.h>

extern const char* APP_NAME;
extern const uint32_t WIDTH;
extern const uint32_t HEIGHT;

class RenderEngine {
public:
    void init();
    void mainLoop();
private:
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkSurfaceKHR surface;
    void initWindow();
    void initInstance();
    void initDevice();
    void initLogicalDevice();
    void initVulkan();
    void cleanup();
};