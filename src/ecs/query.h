#pragma once

#include <memory>
#include <tuple>
#include <type_traits>
#include <vector>

#include "component.h"

template <typename... Args>
class Query {
  static_assert(std::conjunction_v<std::is_base_of<ComponentBase, Args>...>,
                "All target arguments must be Components");

 public:
  using Types = std::tuple<Args...>;
  using ComponentTuple = std::tuple<std::shared_ptr<Args>...>;

  Query(std::vector<ComponentTuple> results)
      : results{results} {

        };

  auto begin() { return results.begin(); }
  auto end() { return results.end(); }
  auto begin() const { return results.begin(); }
  auto end() const { return results.end(); }

  size_t size() const { return results.size(); }
  bool empty() const { return results.empty(); }

 private:
  std::vector<ComponentTuple> results;
};

template <typename T>
concept IsQuery = requires(T& t) { []<typename U>(const Query<U>&) {}(t); };