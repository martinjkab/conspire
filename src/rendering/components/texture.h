#pragma once

#include <glm/vec3.hpp>

#include "ecs/ecs.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

#include "rendering/asset_store.h"
#include "rendering/texture_buffer.h"

struct Texture : ComponentBase {
  Texture(AssetHandle<TextureBuffer> handle) : handle{handle} {}

  friend std::ostream& operator<<(std::ostream& os, const Texture& comp) {
    (void)comp;
    os << "Texture(" << comp.handle.id << ")";
    return os;
  }

  AssetHandle<TextureBuffer> getHandle() { return handle; }

 private:
  AssetHandle<TextureBuffer> handle;
};