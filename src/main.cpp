#include <GLFW/glfw3.h>

#include <iostream>
#include <thread>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "app.h"
#include "ecs/query.h"
#include "rendering/components/transform.h"
#include "rendering/engine.h"
#include "rendering/asset_store.h"
#include "rendering/components/mesh.h"
#include "rendering/render_list.h"

int main() {
  auto engine = std::make_shared<RenderEngine>(RenderEngine{});
  engine->init();

  auto assetStore = AssetStore{};
  auto mesh = Mesh{assetStore.add(engine->uploadMesh(
      {{glm::vec4{-0.5f, -0.5f, 0.0f, 1.0f}, glm::vec2{0.0f, 0.0f}},
       {glm::vec4{0.5f, -0.5f, 0.0f, 1.0f}, glm::vec2{1.0f, 0.0f}},
       {glm::vec4{0.5f, 0.5f, 0.0f, 1.0f}, glm::vec2{1.0f, 1.0f}},
       {glm::vec4{-0.5f, 0.5f, 0.0f, 1.0f}, glm::vec2{0.0f, 1.0f}}},
      {0, 1, 2, 2, 3, 0}))};

  World world{};
  world.addResource(assetStore);
  world.addResource(RenderList{});
  world.addEntity(
      Transform{glm::translate(glm::mat4{1.0}, glm::vec3{-0.5, -0.5, 0.0})},
      mesh);
  world.addEntity(
      Transform{glm::translate(glm::mat4{1.0}, glm::vec3{0.5, 0.5, 0.0})},
      mesh);
  // world.addSystem([](Query<Transform> transformQuery) {
  //   for (auto transformTuple : transformQuery) {
  //     const auto [transform] = transformTuple;
  //     transform->model =
  //         glm::translate(transform->model, glm::vec3{-0.005, -0.005, 0.0});
  //   }
  // });
  world.addSystem([](Query<Transform, Mesh> transformQuery,
                     Resource<RenderList> renderList) {
    renderList->items.clear();
    for (auto transformTuple : transformQuery) {
      const auto [transform, mesh] = transformTuple;
      renderList->items.push_back(
          RenderListItem{mesh->getHandle(), transform->model});
    }
  });
  world.addSystem(
      [&](Resource<RenderList> renderList, Resource<AssetStore> assetStore) {
        engine->mainLoop(*assetStore, *renderList);
      });

  try {
    while (!world.shouldQuit) {
      auto begin = std::chrono::high_resolution_clock::now();
      world.runSystems();
      auto end = std::chrono::high_resolution_clock::now();
      auto diff = begin - end;
      auto sleep = std::chrono::milliseconds(17) - diff;
      if (sleep.count() > 0) {
        std::this_thread::sleep_for(sleep);
      }
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
