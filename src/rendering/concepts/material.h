#pragma once

#include <rendering/materials/material_context.h>

#include <concepts>
#include <type_traits>

template <typename T>
concept Material = requires(const T& m, const MaterialContext& context) {
  { m.uploadUniforms(context) } -> std::same_as<void>;
};