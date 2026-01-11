#pragma once

#include <ecs/ecs.h>

#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <ecs/asset_store.h>
#include <rendering/mesh_buffer.h>

#include <glm/gtx/io.hpp>

struct Mesh : ComponentBase {
  Mesh(AssetHandle<MeshBuffer> handle) : handle{handle} {}

  friend std::ostream& operator<<(std::ostream& os, const Mesh& comp) {
    (void)comp;
    os << "Mesh(" << comp.handle.id << ")";
    return os;
  }

  AssetHandle<MeshBuffer> getHandle() const { return handle; }

 private:
  AssetHandle<MeshBuffer> handle;
};