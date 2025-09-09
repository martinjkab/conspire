#pragma once

#include <type_traits>
#include <tuple>
#include <vector>
#include <memory>
#include "component.h"

struct QueryBase
{
};

template <typename... Args>
class Query : public QueryBase
{
    static_assert(std::conjunction_v<std::is_base_of<Component, Args>...>,
                  "All target arguments must be Components");
public:
    using Types = std::tuple<Args...>;
    using ComponentTuple = std::tuple<std::shared_ptr<Args>...>;

    Query(std::vector<ComponentTuple> results) : results{ results } {

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