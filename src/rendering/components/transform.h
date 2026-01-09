#pragma once

#include <ecs/ecs.h>

#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

struct Transform : ComponentBase {
  glm::mat4 model;

  Transform(const glm::mat4& model = glm::mat4{}) : model{model} {}

  friend std::ostream& operator<<(std::ostream& os, const Transform& comp) {
    (void)comp;
    os << "Transform(" << comp.model << ")";
    return os;
  }
};