#pragma once

#include <rendering/concepts/gpu_aligned.h>

#include <glm/glm.hpp>

struct alignas(16) GlobalUniform {
  glm::mat4 viewProjection;
};

static_assert(GPUAligned<GlobalUniform>);