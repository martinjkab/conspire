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

#include "utils/function_traits.h"
#include "utils/type_map.h"
#include "component.h"
#include "query.h"

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
        std::apply([this, entity_id](auto &&...args_pack)
                   { (([&]
                       {
                using T = std::decay_t<decltype(args_pack)>;
                auto &compVec = components.get<T>();
                compVec.push_back(std::make_shared<T>(args_pack));
                entityToComponentIndex.get<T>()[entity_id] = compVec.size() - 1;
                auto &indexMap = entityToComponentIndex.get<T>();

                std::cout << "Component type: " << typeid(T).name() << "\n";
                std::cout << "Components vector size: " << compVec.size() << "\n";
                std::cout << "EntityToComponentIndex: ";
                for (const auto &pair : indexMap)
                {
                    std::cout << "[" << pair.first << "]=" << pair.second << " ";
                }
                std::cout << "\n"; }()),
                      ...); }, std::forward_as_tuple(args...));
    }

    template <typename Func>
    void addSystem(Func &&system)
    {
        using FuncTraits = function_traits<std::decay_t<Func>>;
        addSystemImpl(std::forward<Func>(system),
                      std::make_index_sequence<FuncTraits::arity>{});
    }

    void runSystems()
    {
        for (auto system : systems)
        {
            system();
        }
    }

private:
    int entityCounter = 0;
    TypeMap<Component, std::vector<std::shared_ptr<Component>>> components;
    TypeMap<Component, std::unordered_map<int, size_t>> entityToComponentIndex;
    std::vector<std::function<void()>> systems;

    template <typename Func, std::size_t... Is>
    void addSystemImpl(Func &&system, std::index_sequence<Is...>)
    {
        using FuncTraits = function_traits<std::decay_t<Func>>;
        static_assert((std::is_base_of_v<QueryBase,
                                         typename FuncTraits::template arg<Is>> &&
                       ...),
                      "All arguments must be Queries");

        systems.emplace_back([this, system = std::forward<Func>(system)]() mutable
                             { system(getQuery<typename FuncTraits::template arg<Is>>()...); });

        auto printComponentTypes = []<typename QueryType>()
        {
            using ComponentTuple = typename QueryType::Types;
            std::apply([](auto &&...args)
                       { ((std::cout << "Component in Query: " << typeid(args).name() << std::endl), ...); }, ComponentTuple{});
        };

        (printComponentTypes.template operator()<typename FuncTraits::template arg<Is>>(), ...);
    }

    template <typename Query>
    Query getQuery()
    {
        static_assert(std::is_base_of_v<QueryBase, Query>, "Argument must be a Query");
        using ComponentTuple = typename Query::Types;
        std::apply([](auto &&...args)
                   { ((std::cout << "Component in getQuery: " << typeid(args).name() << std::endl), ...); }, ComponentTuple{});
        return Query();
    }
};