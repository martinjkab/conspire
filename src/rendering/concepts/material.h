#pragma once

#include <concepts>
#include <type_traits>

template <typename T>
concept Material = requires(const T& m) {
  { m.uploadUniforms() } -> std::same_as<void>;
};