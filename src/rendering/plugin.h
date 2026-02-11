#pragma once

#include "concepts/material.h"
#include "texture.h"
#include <components/mesh.h>
#include <components/transform.h>
#include <core/window_handle.h>
#include <ecs/plugin.h>
#include <input/input_state.h>
#include <input/mouse/mouse_motion.h>
#include <rendering/components/cameras/perspective_camera.h>
#include <rendering/components/mesh_material.h>
#include <rendering/engine.h>
#include <rendering/materials/standard_material.h>

struct RenderPlugin : public Plugin {
  void onAdd(App& app) const override {
    app.addResource(RenderEngine{})
        .addResource(RenderList<StandardMaterial>{})
        .addSystem(
            STARTUP,
            [](Resource<AssetStore> store) { store->add_type<MeshBuffer>(); })
        .addSystem(
            STARTUP,
            [](Resource<AssetStore> store) { store->add_type<Texture>(); })
        .addSystem(STARTUP,
                   [](Resource<AssetStore> store) {
                     store->add_type<StandardMaterial>();
                   })
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
            [](Resource<AssetStore> store, Resource<RenderEngine> engine) {
              auto ctx = engine->getRenderContext();
              auto uploader = engine->getUploader();
              store->loadStaged<Texture>(ctx, uploader);
              store->loadStaged<MeshBuffer>(ctx, uploader);
            })
        .addSystem(UPDATE,
                   [](Query<Transform, Mesh, MeshMaterial<StandardMaterial>>
                          transformQuery,
                      Resource<RenderList<StandardMaterial>> renderList) {
                     renderList->items.clear();
                     for (auto transformTuple : transformQuery) {
                       const auto [transform, mesh, material] = transformTuple;
                       renderList->items.push_back(
                           RenderListItem{mesh->getHandle(), material->material,
                                          transform->model});
                     }
                   })
        .addSystem(UPDATE,
                   [](Resource<RenderList<StandardMaterial>> renderList,
                      Resource<AssetStore> assetStore,
                      Resource<RenderEngine> engine,
                      Query<Transform, PerspectiveCamera> cameraQuery) {
                     const auto [transform, camera] = *(cameraQuery.begin());
                     engine->mainLoop(*assetStore, *renderList,
                                      camera->projection() *
                                          glm::inverse(transform->model));
                   })
        .addSystem(UPDATE, [](Resource<InputState> inputState) {
          inputState->pressed.reset();
          inputState->released.reset();
        });
  }
};