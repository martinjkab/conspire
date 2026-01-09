#pragma once

#include <ecs/ecs.h>

#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <ecs/asset_store.h>
#include <rendering/texture_buffer.h>

#include <glm/gtx/io.hpp>

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