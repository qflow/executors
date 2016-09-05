#ifndef APPLY_H
#define APPLY_H

#include <tuple>
#include <executors/util/function_traits.h>

template <class Tuple, class F, size_t... Is>
constexpr auto apply_impl(Tuple t, F f,
                          std::index_sequence<Is...>) {
    return f(std::get<Is>(t)...);
}

template <class Tuple, class F>
constexpr auto apply(Tuple t, F f) {
    return apply_impl(
        t, f, std::make_index_sequence<std::tuple_size<Tuple>::value>{});
}
#endif
