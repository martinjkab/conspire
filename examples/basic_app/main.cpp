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
#include "input/input_state.h"
#include "ecs/app.h"
#include "rendering/plugin.h"
#include <input/plugin.h>
#include <core/plugin.h>

int main() {
  App{}
      .addResource(AssetStore{})
      .addPlugin(CorePlugin{})
      .addPlugin(RenderPlugin{})
      .addPlugin(InputPlugin{})
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
      .addSystem(UPDATE,
                 [](Query<Transform, PerspectiveCamera> transformQuery,
                    Resource<InputState> inputState) {
                   for (auto transformTuple : transformQuery) {
                     const auto [transform, camera] = transformTuple;
                     glm::vec3 direction{0.0f};
                     const float speed = 1.0f;

                     if (inputState->down.test(GLFW_KEY_W)) {
                       direction.z -= 1.0f;
                     }
                     if (inputState->down.test(GLFW_KEY_S)) {
                       direction.z += 1.0f;
                     }
                     if (inputState->down.test(GLFW_KEY_A)) {
                       direction.x -= 1.0f;
                     }
                     if (inputState->down.test(GLFW_KEY_D)) {
                       direction.x += 1.0f;
                     }
                     if (inputState->down.test(GLFW_KEY_SPACE)) {
                       direction.y += 1.0f;
                     }
                     if (inputState->down.test(GLFW_KEY_LEFT_SHIFT)) {
                       direction.y -= 1.0f;
                     }

                     if (glm::length(direction) > 0.0f) {
                       direction = glm::normalize(direction);
                       glm::mat3 rotation = glm::mat3(transform->model);
                       glm::vec3 worldDirection = rotation * direction;
                       transform->model = glm::translate(
                           transform->model, worldDirection * speed);
                     }
                   }
                 })
      .addSystem(UPDATE,
                 [](Query<Transform, Mesh> transformQuery) {
                   for (auto transformTuple : transformQuery) {
                     const auto [transform, mesh] = transformTuple;
                     transform->model =
                         glm::rotate(transform->model, glm::radians(10.0f),
                                     glm::vec3(0.0f, 1.0f, 0.0f));
                   }
                 })
      .addSystem(UPDATE,
                 [](Resource<EventStore<MouseMotion>> mouseMotionStore) {
                   for (auto event : *mouseMotionStore) {
                     std::cout << "(" << event.delta << ")" << std::endl;
                   }
                 })
      .run();

  return EXIT_SUCCESS;
}