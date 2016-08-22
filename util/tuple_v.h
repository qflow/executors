#ifndef TUPLE_V_H
#define TUPLE_V_H

#include <type_traits>
#include <tuple>

template<typename T>
struct tuple_v
{
    using type = std::tuple<T>;
    type create(T&& arg)
    {
        return type(std::forward<T>(arg));
    }
    type create()
    {
        return type();
    }
};
template<>
struct tuple_v<void>
{
    using type = std::tuple<>;
    type create()
    {
        return type();
    }
};
template<>
struct tuple_v<char>
{
    using type = std::tuple<>;
    type create(char)
    {
        return type();
    }
    type create()
    {
        return type();
    }
};


template<typename... T>
constexpr auto make_empty_tuple_v() -> decltype (std::tuple_cat(tuple_v<T>().create()...))
{
    return std::tuple_cat(tuple_v<T>().create()...);
}
template<typename... T>
constexpr auto make_tuple_v(T&&... args) -> decltype (std::tuple_cat(tuple_v<T>().create(std::forward<T>(args))...))
{
    return std::tuple_cat(tuple_v<T>().create(std::forward<T>(args))...);
}

#endif
