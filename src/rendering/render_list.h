#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "texture_buffer.h"

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