#pragma once

#include "ecs/plugin.h"
#include "rendering/engine.h"
#include "rendering/components/cameras/perspective_camera.h"
#include "components/transform.h"
#include "components/mesh.h"
#include "components/texture.h"
#include <ecs/input_state.h>

struct RenderPlugin : public Plugin {
  void onAdd(App& app) const override {
    app.addResource(RenderEngine{})
        .addResource(RenderList{})
        .addResource(InputState{})
        .addSystem(STARTUP,
                   [](Resource<RenderEngine> engine) { engine->init(); })
        .addSystem(STARTUP,
                   [](World& world) {
                     world.addEntity(
                         Transform{glm::mat4(1.0f)},
                         PerspectiveCamera{35.0f, 1.0f, 0.1f, 1000.0f});
                   })
        .addSystem(
            STARTUP,
            [](Resource<RenderEngine> engine, Resource<InputState> state) {
              glfwSetWindowUserPointer(engine->getWindow(), state.get());

              engine->setKeyCallback([](GLFWwindow* window, int key,
                                        int scancode, int action, int mods) {
                auto* state =
                    static_cast<InputState*>(glfwGetWindowUserPointer(window));
                if (key < 0 || key >= 256) return;

                if (action == GLFW_PRESS) {
                  state->pressed.set(key);
                  state->down.set(key);
                } else if (action == GLFW_RELEASE) {
                  state->released.set(key);
                  state->down.reset(key);
                }
              });
            })
        .addSystem(
            UPDATE,
            [](Query<Transform, Mesh, Texture> transformQuery,
               Resource<RenderList> renderList) {
              renderList->items.clear();
              for (auto transformTuple : transformQuery) {
                const auto [transform, mesh, texture] = transformTuple;
                renderList->items.push_back(RenderListItem{
                    mesh->getHandle(), texture->getHandle(), transform->model});
              }
            })
        .addSystem(
            UPDATE,
            [](Resource<RenderList> renderList, Resource<AssetStore> assetStore,
               Resource<RenderEngine> engine,
               Query<Transform, PerspectiveCamera> cameraQuery) {
              const auto [transform, camera] = *(cameraQuery.begin());
              engine->mainLoop(*assetStore, *renderList,
                               transform->model * camera->projection());
            })
        .addSystem(UPDATE, [](Resource<InputState> inputState) {
          inputState->pressed.reset();
          inputState->released.reset();
        });
  }
};