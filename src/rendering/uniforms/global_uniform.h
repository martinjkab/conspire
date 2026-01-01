#pragma once

#include <glm/glm.hpp>
#include "rendering/concepts/gpu_aligned.h"

struct alignas(16) GlobalUniform {
  glm::mat4 viewProjection;
};

static_assert(GPUAligned<GlobalUniform>);