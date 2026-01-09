#pragma once

#include <memory>
#include <unordered_map>

template <typename T>
struct AssetHandle {
  uint64_t id;
};

class AssetStore {
 public:
  AssetStore() {};

  template <typename T>
  AssetHandle<T> add(T buffer) {
    counter<T> += 1;
    storage<T>[counter<T>] = buffer;
    return AssetHandle<T>{counter<T>};
  }

  template <typename T>
  T operator[](AssetHandle<T> handle) const {
    return storage<T>.at(handle.id);
  }

 private:
  template <typename T>
  static std::unordered_map<uint64_t, T> storage;

  template <typename T>
  static uint64_t counter;
};

template <typename T>
std::unordered_map<uint64_t, T> AssetStore::storage = {};

template <typename T>
uint64_t AssetStore::counter = 1;