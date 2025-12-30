#pragma once

#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include <memory>

#include "mesh_buffer.h"

class MeshStore {
 public:
  MeshStore() {};

  int add(MeshBuffer buffer);

  MeshBuffer operator[](int handle) const;

 private:
  std::unordered_map<int, MeshBuffer> meshes;
  int counter = 0;
};