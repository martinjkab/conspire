#pragma once

#include <concepts>
#include <type_traits>

template <typename T>
concept GPUAligned = std::is_trivially_copyable_v<T> && (alignof(T) >= 16) &&
                     std::is_standard_layout_v<T> && (sizeof(T) % 16 == 0);