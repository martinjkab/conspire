#pragma once

#include <ecs/asset_handle.h>

#include <concepts>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

template <typename T>
class AssetLoader {
 public:
  T load(const std::string& path);
};