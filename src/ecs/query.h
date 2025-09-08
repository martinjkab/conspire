#pragma once

#include <type_traits>
#include <tuple>
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
};