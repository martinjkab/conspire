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
#include "rendering/components/texture.h"

int main() {
  World world{};

  world.addResource(RenderEngine{});
  world.addResource(AssetStore{});
  world.addResource(RenderList{});

  world.addSystem(STARTUP,
                  [](Resource<RenderEngine> engine) { engine->init(); });
  world.addSystem(STARTUP, [](Resource<RenderEngine> engine,
                              Resource<AssetStore> assetStore, World& world) {
    auto meshHandle = engine->uploadMesh(
        {{glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}, glm::vec2{0.0f, 0.0f}},
         {glm::vec4{1.0f, 0.0f, 0.0f, 1.0f}, glm::vec2{1.0f, 0.0f}},
         {glm::vec4{1.0f, 1.0f, 0.0f, 1.0f}, glm::vec2{1.0f, 1.0f}},
         {glm::vec4{0.0f, 1.0f, 0.0f, 1.0f}, glm::vec2{0.0f, 1.0f}}},
        {0, 1, 2, 2, 3, 0});

    auto textureHandle = engine->uploadTexture("assets/sprites/rock.png");

    auto mesh = Mesh{assetStore->add(meshHandle)};
    auto texture = Texture{assetStore->add(textureHandle)};

    world.addEntity(
        Transform{glm::translate(glm::mat4{1.0}, glm::vec3{0, 0, 0.0})}, mesh,
        texture);
  });

  world.addSystem(UPDATE, [](Query<Transform, Mesh, Texture> transformQuery,
                             Resource<RenderList> renderList) {
    renderList->items.clear();
    for (auto transformTuple : transformQuery) {
      const auto [transform, mesh, texture] = transformTuple;
      renderList->items.push_back(RenderListItem{
          mesh->getHandle(), texture->getHandle(), transform->model});
    }
  });

  world.addSystem(UPDATE, [](Resource<RenderList> renderList,
                             Resource<AssetStore> assetStore,
                             Resource<RenderEngine> engine) {
    engine->mainLoop(*assetStore, *renderList);
  });

  world.runSystems(STARTUP);

  try {
    while (!world.shouldQuit) {
      auto begin = std::chrono::high_resolution_clock::now();

      world.runSystems(UPDATE);

      auto end = std::chrono::high_resolution_clock::now();
      auto duration =
          std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
      auto sleep = std::chrono::milliseconds(17) - duration;

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