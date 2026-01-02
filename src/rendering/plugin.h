#pragma once

#include "ecs/plugin.h"
#include "engine.h"

struct RenderPlugin : public Plugin {
  void onAdd(App& app) const override {
    app.addResource(RenderEngine{})
        .addResource(RenderList{})
        .addResource(InputState{})
        .addSystem(STARTUP,
                   [](Resource<RenderEngine> engine) { engine->init(); })
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
               Resource<RenderEngine> engine) {
              engine->mainLoop(*assetStore, *renderList);
            })
        .addSystem(UPDATE, [](Resource<InputState> inputState) {
          inputState->pressed.reset();
          inputState->released.reset();
        });
  }
};