#pragma once

#include "ecs/plugin.h"
#include "rendering/components/cameras/perspective_camera.h"
#include <input/input_state.h>
#include <input/mouse/mouse_motion.h>
#include "window_handle.h"

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

struct CorePlugin : public Plugin {
  void onAdd(App& app) const override {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    auto window = glfwCreateWindow(800, 600, "CONSPIRE", nullptr, nullptr);

    app.addResource(WindowHandle{window});
  }
};