#pragma once

#include "ecs/plugin.h"
#include "rendering/components/cameras/perspective_camera.h"
#include <input/input_state.h>
#include <input/mouse/mouse_motion.h>
#include "core/window_handle.h"

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
#include "input_context.h"
#include "mouse/cursor_position.h"

struct InputPlugin : public Plugin {
  void onAdd(App& app) const override {
    app.addResource(InputContext{})
        .addResource(InputState{})
        .addResource(CursorPosition{})
        .addEvent<MouseMotion>()
        .addSystem(
            STARTUP,
            [](Resource<WindowHandle> windowHandle, Resource<InputState> state,
               Resource<EventStore<MouseMotion>> mouseMotionEventStore,
               Resource<CursorPosition> cursorPosition,
               Resource<InputContext> context) {
              context->inputState = state.get();
              context->mouseMotionEvents = mouseMotionEventStore.get();
              context->cursorPosition = cursorPosition.get();

              glfwSetWindowUserPointer(windowHandle->window, context.get());

              glfwSetKeyCallback(windowHandle->window,
                                 [](GLFWwindow* window, int key, int scancode,
                                    int action, int mods) {
                                   auto* context = static_cast<InputContext*>(
                                       glfwGetWindowUserPointer(window));
                                   auto* state = context->inputState;

                                   if (key < 0 || key >= 256) return;

                                   if (action == GLFW_PRESS) {
                                     state->pressed.set(key);
                                     state->down.set(key);
                                   } else if (action == GLFW_RELEASE) {
                                     state->released.set(key);
                                     state->down.reset(key);
                                   }
                                 });
              glfwSetCursorPosCallback(
                  windowHandle->window,
                  [](GLFWwindow* window, double x, double y) {
                    auto* context = static_cast<InputContext*>(
                        glfwGetWindowUserPointer(window));
                    auto* store = context->mouseMotionEvents;

                    auto lastPos = context->cursorPosition->position;
                    auto newPos = glm::vec2{x, y};

                    store->addEvent(MouseMotion(newPos - lastPos));

                    context->cursorPosition->position = newPos;
                  });
            });
  }
};