#include <GLFW/glfw3.h>

#include <iostream>
#include <thread>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "app.h"
#include "ecs/query.h"
#include "rendering/components/transform.h"
#include "rendering/engine.h"
#include "rendering/mesh_store.h"
#include "rendering/components/mesh.h"

int main() {
  Conspire app;
  World world{};

  Transform transform{};
  auto engine = std::make_shared<RenderEngine>(RenderEngine{});
  engine->init();

  auto meshStore = MeshStore{engine};
  auto mesh = Mesh{meshStore.add(
      {glm::vec4{-0.5f, -0.5f, 0.0f, 1.0f}, glm::vec4{0.5f, -0.5f, 0.0f, 1.0f},
       glm::vec4{0.5f, 0.5f, 0.0f, 1.0f}, glm::vec4{-0.5f, 0.5f, 0.0f, 1.0f}},
      {0, 1, 2, 2, 3, 0})};

  world.addEntity(transform, mesh);
  world.addSystem([](Query<Transform> transformQuery) {
    for (auto transformTuple : transformQuery) {
      const auto [transform] = transformTuple;
      std::cout << *transform << std::endl;
    }
  });
  world.addSystem([&]() { engine->mainLoop(); });
  world.addResource(meshStore);

  try {
    while (!world.shouldQuit) {
      auto begin = std::chrono::high_resolution_clock::now();
      world.runSystems();
      auto end = std::chrono::high_resolution_clock::now();
      auto diff = begin - end;
      auto sleep = std::chrono::milliseconds(1000) - diff;
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
