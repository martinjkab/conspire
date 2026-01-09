#pragma once

#include <core/platform.h>
#include <core/window_handle.h>
#include <ecs/plugin.h>
#include <input/input_state.h>
#include <input/mouse/mouse_motion.h>
#include <input_context.h>
#include <mouse/cursor_position.h>
#include <rendering/components/cameras/perspective_camera.h>

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