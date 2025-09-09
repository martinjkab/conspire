#pragma once

#include <memory>
#include <vector>
#include <type_traits>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <typeindex>
#include <map>
#include <tuple>

template <typename TypeBase, typename Value>
class TypeMap
{
    std::unordered_map<std::type_index, Value> data;

public:
    using iterator = typename std::unordered_map<std::type_index, Value>::iterator;
    using const_iterator = typename std::unordered_map<std::type_index, Value>::const_iterator;

    iterator begin() { return data.begin(); }
    iterator end() { return data.end(); }

    const_iterator begin() const { return data.begin(); }
    const_iterator end() const { return data.end(); }

    const_iterator cbegin() const { return data.cbegin(); }
    const_iterator cend() const { return data.cend(); }

    // Convenience helpers
    std::size_t size() const noexcept { return data.size(); }
    bool empty() const noexcept { return data.empty(); }

    template <typename T>
    void set(Value value)
    {
        static_assert(std::is_base_of<TypeBase, T>::value,
                      "T must be derived from TypeBase");
        data[typeid(T)] = value;
    }

    template <typename T>
    Value &get()
    {
        static_assert(std::is_base_of<TypeBase, T>::value,
                      "T must be derived from TypeBase");
        auto it = data.find(typeid(T));
        if (it == data.end())
        {
            it = data.emplace(typeid(T), Value{}).first;
        }
        return it->second;
    }
};