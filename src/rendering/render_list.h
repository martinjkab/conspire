#pragma once

#include <ecs/asset_store.h>
#include <rendering/concepts/material.h>
#include <rendering/mesh_buffer.h>
#include <rendering/texture.h>

#include <glm/glm.hpp>
#include <vector>

template <Material M>
struct RenderListItem {
  AssetHandle<MeshBuffer> mesh;
  AssetHandle<M> material;
  glm::mat4 model;
};

template <Material M>
class RenderList {
 public:
  std::vector<RenderListItem<M>> items;

 private:
};