#pragma once

#include <glm/glm.hpp>
#include "concepts/gpu_aligned.h"

struct alignas(16) Vertex {
  glm::vec4 position;
  glm::vec2 tex;
};

static_assert(GPUAligned<Vertex>);