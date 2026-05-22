#include "debug/debug_draw_plugin.h"
#include <GLFW/glfw3.h>
#include <core/plugin.h>
#include <ecs/app.h>
#include <ecs/asset_store.h>
#include <ecs/query.h>
#include <input/input_state.h>
#include <input/plugin.h>
#include <rendering/components/mesh.h>
#include <rendering/components/transform.h>
#include <rendering/engine.h>
#include <rendering/plugin.h>
#include <rendering/render_list.h>

#include <array>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>

namespace {
void drawBoundingBox(DebugDraw& debugDraw, const glm::mat4& model,
                     const MeshBounds& bounds, const glm::vec4& color) {
  const std::array<glm::vec3, 8> localCorners{{
      {bounds.min.x, bounds.min.y, bounds.min.z},
      {bounds.max.x, bounds.min.y, bounds.min.z},
      {bounds.min.x, bounds.max.y, bounds.min.z},
      {bounds.max.x, bounds.max.y, bounds.min.z},
      {bounds.min.x, bounds.min.y, bounds.max.z},
      {bounds.max.x, bounds.min.y, bounds.max.z},
      {bounds.min.x, bounds.max.y, bounds.max.z},
      {bounds.max.x, bounds.max.y, bounds.max.z},
  }};

  std::array<glm::vec3, 8> worldCorners{};
  for (size_t i = 0; i < localCorners.size(); ++i) {
    worldCorners[i] = glm::vec3{model * glm::vec4{localCorners[i], 1.0f}};
  }

  constexpr std::array<std::pair<size_t, size_t>, 12> edges{{
      {0, 1},
      {1, 3},
      {3, 2},
      {2, 0},
      {4, 5},
      {5, 7},
      {7, 6},
      {6, 4},
      {0, 4},
      {1, 5},
      {2, 6},
      {3, 7},
  }};

  for (const auto& [a, b] : edges) {
    debugDraw.line(worldCorners[a], worldCorners[b], color);
  }
}
}  // namespace

int main() {
  App{}
      .addResource(AssetStore{})
      .addPlugin(CorePlugin{})
      .addPlugin(RenderPlugin{})
      .addPlugin(InputPlugin{})
      .addPlugin(DebugDrawPlugin{})
      .addSystem(STARTUP,
                 [](Resource<WindowHandle> windowHandle) {
                   glfwSetInputMode(windowHandle->window, GLFW_CURSOR,
                                    GLFW_CURSOR_DISABLED);
                 })
      .addSystem(UPDATE,
                 [](App& app, Resource<InputState> inputState) {
                   if (inputState->down.test(GLFW_KEY_ESCAPE)) {
                     app.stop();
                   }
                 })
      .addSystem(STARTUP,
                 [](Resource<RenderEngine> engine,
                    Resource<AssetStore> assetStore, World& world) {
                   auto mesh = Mesh{
                       assetStore->load<MeshBuffer>("assets/models/rat.glb")};
                   auto texture = MeshMaterial<StandardMaterial>{
                       assetStore->add_staged<StandardMaterial>(
                           StandardMaterial{assetStore->load<Texture>(
                               "assets/sprites/rat_albedo.png")})};

                   world.addEntity(Transform{glm::translate(
                                       glm::mat4{1.0}, glm::vec3{0, 0, -75.0})},
                                   mesh, texture);
                 })
      .addSystem(UPDATE,
                 [](Query<Transform, PerspectiveCamera> transformQuery,
                    Resource<InputState> inputState) {
                   for (auto [transform, camera] : transformQuery) {
                     glm::vec3 direction{0.0f};
                     const float speed = 1.0f;

                     glm::vec3 forward =
                         -glm::normalize(glm::vec3(transform->model[2]));
                     glm::vec3 right =
                         glm::normalize(glm::vec3(transform->model[0]));
                     glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

                     if (inputState->down.test(GLFW_KEY_W)) {
                       direction += forward;
                     }
                     if (inputState->down.test(GLFW_KEY_S)) {
                       direction -= forward;
                     }
                     if (inputState->down.test(GLFW_KEY_A)) {
                       direction -= right;
                     }
                     if (inputState->down.test(GLFW_KEY_D)) {
                       direction += right;
                     }
                     if (inputState->down.test(GLFW_KEY_SPACE)) {
                       direction += up;
                     }
                     if (inputState->down.test(GLFW_KEY_LEFT_SHIFT)) {
                       direction -= up;
                     }

                     if (glm::length(direction) > 0.0f) {
                       direction = glm::normalize(direction);
                       transform->model[3] +=
                           glm::vec4(direction * speed, 0.0f);
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
                 [](Resource<AssetStore> assetStore,
                    Resource<DebugDraw> debugDraw,
                    Query<Transform, Mesh> transformQuery) {
                   for (auto [transform, mesh] : transformQuery) {
                     const auto meshBuffer = (*assetStore)[mesh->getHandle()];
                     if (!meshBuffer.has_value()) {
                       continue;
                     }

                     drawBoundingBox(*debugDraw, transform->model,
                                     meshBuffer->bounds, {0, 1, 0, 1});
                   }
                 })
      .addSystem(UPDATE,
                 [](Resource<EventStore<MouseMotion>> mouseMotionStore,
                    Query<Transform, PerspectiveCamera> cameraQuery) {
                   const float sensitivity = 0.1f;

                   for (auto event : *mouseMotionStore) {
                     for (auto [transform, camera] : cameraQuery) {
                       float yaw = glm::radians(-event.delta.x * sensitivity);
                       float pitch = glm::radians(-event.delta.y * sensitivity);

                       glm::vec3 right =
                           glm::normalize(glm::vec3(transform->model[0]));

                       transform->model = glm::rotate(glm::mat4(1.0f), yaw,
                                                      glm::vec3(0, 1, 0)) *
                                          transform->model;

                       transform->model = glm::rotate(transform->model, pitch,
                                                      glm::vec3(1, 0, 0));
                     }
                   }
                 })
      .run();

  return EXIT_SUCCESS;
}
