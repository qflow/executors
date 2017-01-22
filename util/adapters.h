#ifndef ADAPTERS_H
#define ADAPTERS_H

#include <utility>

struct empty{};

template <typename T>
struct map_traits
{};
template<typename Key, typename... Value>
struct map_traits<std::tuple<std::pair<Key, Value>...>>
{
    static constexpr bool is_map = true;
};

template <typename T>
struct tuple_traits
{};

template<typename... T>
struct tuple_traits<std::tuple<T...>>
{
    static constexpr bool is_tuple = true;
};

namespace adapters
{
    template<typename A, typename B, typename Enable = void>
    struct adapter
    {
        static A convert(B b);
    };
    template<typename A, typename B>
    A as(B b)
    {
        return adapter<A, B>::convert(std::forward<B>(b));
    }
}

#endif
