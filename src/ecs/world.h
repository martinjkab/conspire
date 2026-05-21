#pragma once

#include <ecs/component.h>
#include <ecs/phase.h>
#include <ecs/query.h>
#include <ecs/resource.h>
#include <ecs/utils/function_traits.h>
#include <ecs/utils/remove_smart_ptr.h>
#include <ecs/utils/type_map.h>
#include <ecs/utils/vec_to_tuple.h>

#include <any>
#include <memory>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <vector>

class World {
public:
  World() = default;
  ~World() = default;

  template <typename... Args> int addEntity(Args... args) {
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

    return entity_id;
  }

  template <typename T> void addResource(T&& resource) {
    using DecayedT = typename std::decay<T>::type;
    resources[typeid(DecayedT)] = new DecayedT{std::forward<T>(resource)};
  }

  template <typename T>
  void processIntersection(std::vector<int>& intersection, bool& first) {
    using Comp = remove_smart_ptr_t<T>;
    auto& entityMap = entityToComponentIndex.get<Comp>();

    if (first) {
      intersection.reserve(entityMap.size());
      for (const auto& p : entityMap)
        intersection.push_back(p.first);
      first = false;
    } else {
      std::vector<int> next;
      next.reserve(std::min(intersection.size(), entityMap.size()));
      for (int id : intersection) {
        if (entityMap.find(id) != entityMap.end())
          next.push_back(id);
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

  template <typename Query> Query getQuery() {
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

  template <typename Resource> Resource getResource() {
    using ResourceType = typename Resource::Type;
    return Resource{
        std::any_cast<ResourceType*>(resources[typeid(ResourceType)])};
  }

private:
  int entityCounter = 0;
  TypeMap<ComponentBase, std::vector<std::shared_ptr<ComponentBase>>>
      components;
  TypeMap<ComponentBase, std::unordered_map<int, size_t>>
      entityToComponentIndex;
  std::unordered_map<std::type_index, std::any> resources;
};