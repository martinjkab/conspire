#pragma once

#include <ecs/asset_handle.h>
#include <ecs/asset_traits.h>

#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

template <typename T>
class AssetLoader {
 public:
  typename AssetTraits<T>::CPUDataType load(const std::string& path) const;
};