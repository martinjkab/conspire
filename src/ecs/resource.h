#pragma once

#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

#include "component.h"

template <typename T>
class Resource {
 public:
  using Type = T;

  Resource(std::shared_ptr<T> value) : value{value} {};

  T& operator*() { return *value; }

  const T& operator*() const { return *value; }

  T* operator->() { return value.get(); }

  T* get() { return value.get(); }

 private:
  std::shared_ptr<T> value;
};

template <typename T>
concept IsResource =
    requires(T& t) { []<typename U>(const Resource<U>&) {}(t); };