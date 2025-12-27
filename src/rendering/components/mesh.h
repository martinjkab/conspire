#pragma once

#include <glm/vec3.hpp>

#include "ecs/ecs.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

struct Mesh : ComponentBase {
  Mesh(int handle) : handle{handle} {}

  friend std::ostream& operator<<(std::ostream& os, const Mesh& comp) {
    (void)comp;
    os << "Mesh(" << comp.handle << ")";
    return os;
  }

 private:
  int handle;
};