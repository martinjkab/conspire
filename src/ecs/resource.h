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

  std::shared_ptr<T> value;

 private:
};

template <typename T>
concept IsResource =
    requires(T& t) { []<typename U>(const Resource<U>&) {}(t); };