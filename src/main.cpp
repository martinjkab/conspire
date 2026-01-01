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
#include "ecs/input_state.h"

int main() {
  World world{};

  world.addResource(RenderEngine{});
  world.addResource(AssetStore{});
  world.addResource(RenderList{});
  world.addResource(InputState{});

  world.addSystem(STARTUP,
                  [](Resource<RenderEngine> engine) { engine->init(); });

  world.addSystem(STARTUP, [](Resource<RenderEngine> engine,
                              Resource<InputState> state) {
    glfwSetWindowUserPointer(engine->getWindow(), state.get());

    engine->setKeyCallback([](GLFWwindow* window, int key, int scancode,
                              int action, int mods) {
      auto* state = static_cast<InputState*>(glfwGetWindowUserPointer(window));
      if (key < 0 || key >= 256) return;

      if (action == GLFW_PRESS) {
        state->pressed.set(key);
        state->down.set(key);
      } else if (action == GLFW_RELEASE) {
        state->released.set(key);
        state->down.reset(key);
      }
    });
  });

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
  world.addSystem(UPDATE, [](Query<Transform> transformQuery,
                             Resource<InputState> inputState) {
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
      transform->model = glm::translate(transform->model, direction * speed);
    }
  });

  world.addSystem(UPDATE, [](Resource<RenderList> renderList,
                             Resource<AssetStore> assetStore,
                             Resource<RenderEngine> engine) {
    engine->mainLoop(*assetStore, *renderList);
  });
  world.addSystem(UPDATE, [](Resource<InputState> inputState) {
    inputState->pressed.reset();
    inputState->released.reset();
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