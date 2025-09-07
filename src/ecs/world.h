#pragma once

#include <memory>
#include <vector>
#include <type_traits>
#include <functional>
#include <iostream>
#include <unordered_map>
#include <typeindex>
#include <map>

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

class Component
{
public:
    virtual ~Component() = default;

    friend std::ostream &operator<<(std::ostream &os, const Component &comp)
    {
        os << "Component";
        return os;
    }
};

class World
{
public:
    World() = default;
    ~World() = default;

    template <typename... Args>
    void addEntity(Args... args)
    {
        static_assert(std::conjunction_v<std::is_base_of<Component, Args>...>, "All arguments must be Component");
        auto entity_id = entityCounter++;
        (void)std::initializer_list<int>{
            (
                [&]
                {
                    components.get<Args>() = std::vector<std::shared_ptr<Component>>{};
                    entityToComponentIndex.get<Args>()[entity_id] = 0;
                    return 0;
                }(),
                0)...};

        (void)std::initializer_list<int>{
            (
                [&]
                {
                    using T = std::remove_reference_t<Args>;
                    std::cout << "Component type: " << typeid(T).name() << "\n";
                    auto &compVec = components.get<T>();
                    std::cout << "Components vector size: " << compVec.size() << "\n";
                    auto &indexMap = entityToComponentIndex.get<T>();
                    std::cout << "EntityToComponentIndex: ";
                    for (const auto &pair : indexMap)
                    {
                        std::cout << "[" << pair.first << "]=" << pair.second << " ";
                    }
                    std::cout << "\n";
                    return 0;
                }(),
                0)...};
    }

private:
    int entityCounter = 0;
    TypeMap<Component, std::vector<std::shared_ptr<Component>>> components;
    TypeMap<Component, std::unordered_map<int, int>> entityToComponentIndex;
};