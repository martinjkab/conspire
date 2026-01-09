#pragma once

#include <concepts/gpu_aligned.h>

#include <glm/glm.hpp>

struct alignas(16) Vertex {
  glm::vec4 position;
  glm::vec2 tex;
};

static_assert(GPUAligned<Vertex>);