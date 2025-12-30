#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "component.h"
#include "query.h"
#include "resource.h"
#include "utils/function_traits.h"
#include "utils/remove_smart_ptr.h"
#include "utils/type_map.h"
#include "utils/vec_to_tuple.h"

template <typename T>
concept IsSystemParam = (IsQuery<T> || IsResource<T>);

class World {
 public:
  World() = default;
  ~World() = default;

  template <typename... Args>
  void addEntity(Args... args) {
    static_assert(std::conjunction_v<std::is_base_of<ComponentBase, Args>...>,
                  "All arguments must be Component");
    auto entity_id = entityCounter++;
    std::apply(
        [this, entity_id](auto&&... args_pack) {
          (([&] {
             using T = std::decay_t<decltype(args_pack)>;
             auto& compVec = components.get<T>();
             compVec.push_back(std::make_shared<T>(args_pack));
             entityToComponentIndex.get<T>()[entity_id] = compVec.size() - 1;
           }()),
           ...);
        },
        std::forward_as_tuple(args...));
  }

  template <typename Func>
  void addSystem(Func&& system) {
    using FuncTraits = function_traits<std::decay_t<Func>>;
    addSystemImpl(std::forward<Func>(system),
                  std::make_index_sequence<FuncTraits::arity>{});
  }

  template <typename T>
  void addResource(T&& resource) {
    using DecayedT = typename std::decay<T>::type;
    resources[typeid(DecayedT)] =
        std::make_shared<DecayedT>(std::forward<T>(resource));
  }

  void runSystems() {
    for (auto system : systems) {
      system();
    }
  }

  bool shouldQuit = false;

 private:
  int entityCounter = 0;
  TypeMap<ComponentBase, std::vector<std::shared_ptr<ComponentBase>>>
      components;
  TypeMap<ComponentBase, std::unordered_map<int, size_t>>
      entityToComponentIndex;
  std::unordered_map<std::type_index, std::shared_ptr<void>> resources;
  std::vector<std::function<void()>> systems;

  template <typename T>
  void processIntersection(std::vector<int>& intersection, bool& first) {
    using Comp = remove_smart_ptr_t<T>;
    auto& entityMap = entityToComponentIndex.get<Comp>();

    if (first) {
      intersection.reserve(entityMap.size());
      for (const auto& p : entityMap) intersection.push_back(p.first);
      first = false;
    } else {
      std::vector<int> next;
      next.reserve(std::min(intersection.size(), entityMap.size()));
      for (int id : intersection) {
        if (entityMap.find(id) != entityMap.end()) next.push_back(id);
      }
      intersection.swap(next);
    }
  }

  template <typename T>
  std::shared_ptr<remove_smart_ptr_t<T>> getComponentForEntity(int entity) {
    using Comp = remove_smart_ptr_t<T>;
    auto& componentIndex = entityToComponentIndex.get<Comp>()[entity];
    auto& component = components.get<Comp>()[componentIndex];
    return std::static_pointer_cast<Comp>(component);
  }

  template <typename Query>
  Query getQuery() {
    using ComponentTuple = typename Query::Types;
    bool first = true;
    std::vector<int> intersection;

    [this, &intersection,
     &first]<typename... ComponentTypes>(std::tuple<ComponentTypes...>*) {
      (this->processIntersection<ComponentTypes>(intersection, first), ...);
    }(static_cast<ComponentTuple*>(nullptr));

    auto results = std::vector<typename Query::ComponentTuple>();
    for (auto entity : intersection) {
      auto componentTuple = [this, entity]<typename... ComponentTypes>(
                                std::tuple<ComponentTypes...>*) {
        return std::make_tuple(
            this->getComponentForEntity<ComponentTypes>(entity)...);
      }(static_cast<ComponentTuple*>(nullptr));
      results.push_back(componentTuple);
    }
    return Query{results};
  }

  template <typename Resource>
  Resource getResource() {
    using ResourceType = typename Resource::Type;
    return static_pointer_cast<ResourceType>(resources[typeid(ResourceType)]);
  }

  template <typename Func, std::size_t... Is>
  void addSystemImpl(Func&& system, std::index_sequence<Is...>) {
    using FuncTraits = function_traits<std::decay_t<Func>>;
    static_assert((IsSystemParam<typename FuncTraits::template arg<Is>> && ...),
                  "All arguments must be SystemParams");

    systems.emplace_back([this, system = std::forward<Func>(system)]() mutable {
      system([this]() -> decltype(auto) {
        using ArgType = typename FuncTraits::template arg<Is>;
        if constexpr (IsResource<ArgType>) {
          return this->getResource<ArgType>();
        } else {
          return this->getQuery<ArgType>();
        }
      }()...);
    });
  }
};