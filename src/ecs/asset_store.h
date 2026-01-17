#pragma once

#include <ecs/asset_handle.h>
#include <ecs/asset_loader.h>

#include <any>
#include <memory>
#include <thread>
#include <typeindex>
#include <unordered_map>

class AssetStore {
 public:
  AssetStore() {};

  template <typename T>
  AssetHandle<T> load(const std::string& path) {
    counter<T> += 1;
    storage<T>[counter<T>] = std::nullopt;

    const auto loader = std::any_cast<AssetLoader<T>>(loaders[typeid(T)]);
    std::jthread([&loader, &path]() {
      typename AssetTraits<T>::CPUDataType data = loader.load(path);
    });

    return AssetHandle<T>{counter<T>};
  }

  template <typename T>
  std::optional<T> operator[](AssetHandle<T> handle) const {
    return storage<T>.at(handle.id);
  }

 private:
  std::unordered_map<std::type_index, std::any> loaders;

  template <typename T>
  static std::unordered_map<uint64_t, std::optional<T>> storage;

  template <typename T>
  static std::unordered_map<uint64_t, typename AssetTraits<T>::CPUDataType>
      staging_storage;

  template <typename T>
  static uint64_t counter;
};

template <typename T>
std::unordered_map<uint64_t, std::optional<T>> AssetStore::storage = {};

template <typename T>
std::unordered_map<uint64_t, typename AssetTraits<T>::CPUDataType>
    AssetStore::staging_storage = {};

template <typename T>
uint64_t AssetStore::counter = 1;