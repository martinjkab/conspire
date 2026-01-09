#pragma once

#include <rendering/mesh_buffer.h>
#include <rendering/texture_buffer.h>

#include <glm/glm.hpp>
#include <vector>

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