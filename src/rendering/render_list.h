#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "rendering/texture_buffer.h"
#include "rendering/mesh_buffer.h"

struct RenderListItem {
  AssetHandle<MeshBuffer> meshHandle;
  AssetHandle<TextureBuffer> textureHandle;
  glm::mat4 model;
};

class RenderList {
 public:
  std::vector<RenderListItem> items;

 private:
};