#pragma once

#include <ecs/asset_handle.h>
#include <ecs/asset_loader.h>

#include <any>
#include <optional>
#include <thread>
#include <typeindex>
#include <unordered_map>

class AssetStore {
public:
  AssetStore() {};

  template <typename T> AssetHandle<T> load(const std::string& path) {
    counter<T> += 1;
    storage<T>[counter<T>] = std::nullopt;

    std::jthread loadCPUThread([this, &path]() {
      const auto loader = std::any_cast<AssetLoader<T>>(loaders[typeid(T)]);
      const auto data = loader.loadCPU(path);
      staging_storage<T>[counter<T>] = data;
    });

    return AssetHandle<T>{counter<T>};
  }

  template <typename T> void loadStaged() {
    const auto loader = std::any_cast<AssetLoader<T>>(loaders[typeid(T)]);
    for (const auto CPUDataTuple : staging_storage<T>) {
      const auto [id, CPUData] = CPUDataTuple;
      const auto GPUData = loader.loadGPU(CPUData);
      storage<T>[id] = GPUData;
    }
  }

  template <typename T> AssetHandle<T> add(T data) {
    counter<T> += 1;
    storage<T>[counter<T>] = data;
    return AssetHandle<T>{counter<T>};
  }

  template <typename T>
  std::optional<T> operator[](AssetHandle<T> handle) const {
    return storage<T>.at(handle.id);
  }

  template <typename T> void add_type() {
    loaders[typeid(T)] = AssetLoader<T>();
  }

private:
  std::unordered_map<std::type_index, std::any> loaders;

  template <typename T>
  static std::unordered_map<uint64_t, std::optional<T>> storage;

  template <typename T>
  static std::unordered_map<uint64_t, typename AssetTraits<T>::CPUDataType>
      staging_storage;

  template <typename T> static uint64_t counter;
};

template <typename T>
std::unordered_map<uint64_t, std::optional<T>> AssetStore::storage = {};

template <typename T>
std::unordered_map<uint64_t, typename AssetTraits<T>::CPUDataType>
    AssetStore::staging_storage = {};

template <typename T> uint64_t AssetStore::counter = 0;