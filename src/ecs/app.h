#pragma once

#include <ecs/event_store.h>
#include <ecs/plugin.h>
#include <ecs/world.h>

#include <type_traits>

struct Plugin;

template <typename T>
concept IsSystemParam =
    (IsQuery<T> || IsResource<T> || std::is_same_v<T, World&>);

class App {
 public:
  void run();

  void runSystems(Phase phase);

  template <typename Func>
  App& addSystem(Phase phase, Func&& system) {
    using FuncTraits = function_traits<std::decay_t<Func>>;
    addSystemImpl(phase, std::forward<Func>(system),
                  std::make_index_sequence<FuncTraits::arity>{});

    return *this;
  }

  template <typename T>
  App& addResource(T&& resource) {
    _world.addResource(resource);

    return *this;
  }

  template <typename T>
  App& addEvent() {
    _world.addResource(EventStore<T>());
    addSystem(POST_UPDATE,
              [](Resource<EventStore<T>> eventStore) { eventStore->update(); });
    return *this;
  }

  App& addPlugin(const Plugin& plugin);

 private:
  World _world;
  std::unordered_map<Phase, std::vector<std::function<void()>>> systems;
  bool running = true;

  template <typename Func, std::size_t... Is>
  void addSystemImpl(Phase phase, Func&& system, std::index_sequence<Is...>) {
    using FuncTraits = function_traits<std::decay_t<Func>>;
    static_assert((IsSystemParam<typename FuncTraits::template arg<Is>> && ...),
                  "All arguments must be SystemParams");

    systems[phase].emplace_back(
        [this, system = std::forward<Func>(system)]() mutable {
          system([&]() -> decltype(auto) {
            using ArgType = typename FuncTraits::template arg<Is>;

            if constexpr (IsResource<ArgType>) {
              return this->_world.getResource<ArgType>();
            } else if constexpr (IsQuery<ArgType>) {
              return this->_world.getQuery<ArgType>();
            } else {
              return (this->_world);
            }
          }()...);
        });
  }
};