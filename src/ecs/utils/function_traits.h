template <typename T>
struct function_traits;

template <typename R, typename... Args>
struct function_traits<R(Args...)>
{
    using return_type = R;
    static constexpr std::size_t arity = sizeof...(Args);

    template <std::size_t N>
    using arg = std::tuple_element_t<N, std::tuple<Args...>>;
};

template <typename T>
struct function_traits : function_traits<decltype(&T::operator())>
{
};

template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...) const>
{
    using return_type = R;
    static constexpr std::size_t arity = sizeof...(Args);

    template <std::size_t N>
    using arg = std::tuple_element_t<N, std::tuple<Args...>>;
};

template <typename C, typename R, typename... Args>
struct function_traits<R (C::*)(Args...)> : function_traits<R (C::*)(Args...) const>
{
};