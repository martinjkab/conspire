#pragma once

#include <glm/glm.hpp>

struct Camera {
  virtual glm::mat4 projection() const = 0;
};