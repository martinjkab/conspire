#include <GLFW/glfw3.h>

#include <iostream>
#include <thread>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "ecs/app.h"
#include "ecs/query.h"
#include "rendering/components/transform.h"
#include "rendering/engine.h"
#include "ecs/asset_store.h"
#include "rendering/components/mesh.h"
#include "rendering/render_list.h"
#include "rendering/components/texture.h"
#include "ecs/input_state.h"
#include "ecs/app.h"
#include "rendering/plugin.h"

int main() {
  App{}
      .addResource(AssetStore{})
      .addPlugin(RenderPlugin{})
      .addSystem(STARTUP,
                 [](Resource<RenderEngine> engine,
                    Resource<AssetStore> assetStore, World& world) {
                   auto meshHandle =
                       engine->uploadGltf("assets/models/rat.glb");

                   auto textureHandle =
                       engine->uploadTexture("assets/sprites/rat_albedo.png");

                   auto mesh = Mesh{assetStore->add(meshHandle)};
                   auto texture = Texture{assetStore->add(textureHandle)};

                   world.addEntity(Transform{glm::translate(
                                       glm::mat4{1.0}, glm::vec3{0, 0, -75.0})},
                                   mesh, texture);
                 })
      .addSystem(
          UPDATE,
          [](Query<Transform> transformQuery, Resource<InputState> inputState) {
            for (auto transformTuple : transformQuery) {
              const auto [transform] = transformTuple;
              auto direction = glm::vec3{0};
              const float speed = 10.0f;
              if (inputState->down.test(GLFW_KEY_A)) {
                direction += glm::vec3(-1, 0, 0);
              }
              if (inputState->down.test(GLFW_KEY_D)) {
                direction += glm::vec3(1, 0, 0);
              }
              if (inputState->down.test(GLFW_KEY_W)) {
                direction += glm::vec3(0, -1, 0);
              }
              if (inputState->down.test(GLFW_KEY_S)) {
                direction += glm::vec3(0, 1, 0);
              }
              transform->model =
                  glm::translate(transform->model, direction * speed);
            }
          })
      .addSystem(UPDATE,
                 [](Query<Transform> transformQuery) {
                   for (auto transformTuple : transformQuery) {
                     const auto [transform] = transformTuple;
                     transform->model =
                         glm::rotate(transform->model, glm::radians(10.0f),
                                     glm::vec3(0.0f, 1.0f, 0.0f));
                   }
                 })
      .run();

  return EXIT_SUCCESS;
}