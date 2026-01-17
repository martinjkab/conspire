#pragma once

#include <ecs/asset_handle.h>
#include <ecs/asset_loader.h>

#include <any>
#include <memory>
#include <typeindex>
#include <unordered_map>

class AssetStore {
 public:
  AssetStore() {};

  template <typename T>
  AssetHandle<T> load(const std::string& path) {
    auto loader = std::any_cast<AssetLoader<T>>(loaders[typeid(T)]);
    T data = loader.load(path);

    return add(data);
  }

  template <typename T>
  AssetHandle<T> add(T data) {
    counter<T> += 1;
    storage<T>[counter<T>] = data;
    return AssetHandle<T>{counter<T>};
  }

  template <typename T>
  T operator[](AssetHandle<T> handle) const {
    return storage<T>.at(handle.id);
  }

 private:
  std::unordered_map<std::type_index, std::any> loaders;

  template <typename T>
  static std::unordered_map<uint64_t, T> storage;

  template <typename T>
  static uint64_t counter;
};

template <typename T>
std::unordered_map<uint64_t, T> AssetStore::storage = {};

template <typename T>
uint64_t AssetStore::counter = 1;