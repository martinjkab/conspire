#pragma once

#include <glm/vec3.hpp>

#include "ecs/ecs.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

#include "rendering/asset_store.h"
#include "rendering/mesh_buffer.h"

struct Mesh : ComponentBase {
  Mesh(AssetHandle<MeshBuffer> handle) : handle{handle} {}

  friend std::ostream& operator<<(std::ostream& os, const Mesh& comp) {
    (void)comp;
    os << "Mesh(" << comp.handle.id << ")";
    return os;
  }

  AssetHandle<MeshBuffer> getHandle() { return handle; }

 private:
  AssetHandle<MeshBuffer> handle;
};