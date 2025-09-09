#pragma once

#include <memory>
#include <type_traits>

template <typename T>
struct remove_smart_ptr {
    using type = T;
};

template <typename T>
struct remove_smart_ptr<std::shared_ptr<T>> {
    using type = typename remove_smart_ptr<T>::type;
};

template <typename T, typename D>
struct remove_smart_ptr<std::unique_ptr<T, D>> {
    using type = typename remove_smart_ptr<T>::type;
};

template <typename T>
struct remove_smart_ptr<std::weak_ptr<T>> {
    using type = typename remove_smart_ptr<T>::type;
};

template <typename T>
using remove_smart_ptr_t = typename remove_smart_ptr<T>::type;
