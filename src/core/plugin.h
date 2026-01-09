#pragma once

#include <ecs/plugin.h>
#include <input/input_state.h>
#include <input/mouse/mouse_motion.h>
#include <platform.h>
#include <rendering/components/cameras/perspective_camera.h>
#include <window_handle.h>

struct CorePlugin : public Plugin {
  void onAdd(App& app) const override {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    auto window = glfwCreateWindow(800, 600, "CONSPIRE", nullptr, nullptr);

    app.addResource(WindowHandle{window});
  }
};