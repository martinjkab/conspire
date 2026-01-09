#pragma once

#include "ecs/plugin.h"
#include "rendering/engine.h"
#include "rendering/components/cameras/perspective_camera.h"
#include "components/transform.h"
#include "components/mesh.h"
#include "components/texture.h"
#include <input/input_state.h>
#include <input/mouse/mouse_motion.h>
#include <core/window_handle.h>

struct RenderPlugin : public Plugin {
  void onAdd(App& app) const override {
    app.addResource(RenderEngine{})
        .addResource(RenderList{})
        .addSystem(STARTUP,
                   [](Resource<RenderEngine> engine,
                      Resource<WindowHandle> windowHandle) {
                     engine->init(windowHandle->window);
                   })
        .addSystem(STARTUP,
                   [](World& world) {
                     world.addEntity(Transform{glm::mat4(1.0f)},
                                     PerspectiveCamera{glm::radians(90.0f),
                                                       1.0f, 0.1f, 1000.0f});
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
              engine->mainLoop(
                  *assetStore, *renderList,
                  camera->projection() * glm::inverse(transform->model));
            })
        .addSystem(UPDATE, [](Resource<InputState> inputState) {
          inputState->pressed.reset();
          inputState->released.reset();
        });
  }
};