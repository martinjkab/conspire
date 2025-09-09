#pragma once

#include <tuple>
#include <vector>
#include <utility>
#include <iostream>

template <typename T, std::size_t... Is>
auto vec_to_tuple_impl(const std::vector<T>& v, std::index_sequence<Is...>) {
    return std::make_tuple(v[Is]...);
}

template <std::size_t N, typename T>
auto vec_to_tuple(const std::vector<T>& v) {
    if (v.size() < N) throw std::runtime_error("vector too small");
    return vec_to_tuple_impl(v, std::make_index_sequence<N>{});
}