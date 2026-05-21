#pragma once

#include <core/platform.h>
#include <core/window_handle.h>
#include <ecs/plugin.h>
#include <input/input_state.h>
#include <input/mouse/mouse_motion.h>
#include <rendering/components/cameras/perspective_camera.h>

struct CorePlugin : public Plugin {
  void onAdd(App& app) const override {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    auto window = glfwCreateWindow(800, 600, "CONSPIRE", nullptr, nullptr);

    app.addResource(WindowHandle{window})
        .addSystem(UPDATE, [](App& app, Resource<WindowHandle> windowHandle) {
          glfwPollEvents();
          if (glfwWindowShouldClose(windowHandle->window)) {
            app.stop();
          }
        });
  }
};
