#pragma once

#include <type_traits>
#include <tuple>
#include <vector>
#include <memory>
#include "component.h"

template <typename T>
class Resource
{
public:
    using Type = T;

    Resource(std::shared_ptr<T> value) : value{value} {};

    std::shared_ptr<T> value;

private:
};

template <typename T>
concept IsResource = requires(T &t) {
    []<typename U>(const Resource<U> &) {}(t);
};