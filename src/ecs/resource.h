#pragma once

#include <ecs/component.h>

#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

template <typename T> class Resource {
public:
  using Type = T;

  Resource(T* value) : value{value} {};

  T& operator*() { return *value; }

  const T& operator*() const { return *value; }

  T* operator->() { return value; }

  T* get() { return value; }

private:
  T* value;
};

template <typename T>
concept IsResource =
    requires(T& t) { []<typename U>(const Resource<U>&) {}(t); };