#pragma once

#include <memory>
#include <vector>
#include <type_traits>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <typeindex>
#include <map>
#include <functional>
#include <tuple>

template <typename TypeBase, typename Value>
class TypeMap
{
    std::unordered_map<std::type_index, Value> data;

public:
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