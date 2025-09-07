#pragma once
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "ecs/world.h"

extern const char *APP_NAME;
extern const uint32_t WIDTH;
extern const uint32_t HEIGHT;

class Conspire
{
public:
    void run();

private:
    GLFWwindow *window;
    VkInstance instance;
    void initWindow();
    void initVulkan();
    void initECS();
    void mainLoop();
    void cleanup();
};