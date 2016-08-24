#ifndef FOR_EACH_H
#define FOR_EACH_H

#include <tuple>

template <class Tuple, class F, size_t... Is>
constexpr auto for_each_t_impl(Tuple t, F f,
                          std::index_sequence<Is...>) {
    return std::make_tuple(f(std::integral_constant<size_t, Is>(), std::get<Is>(t))...);
}

template <class Tuple, class F>
constexpr auto for_each_t(Tuple t, F f) {
    return for_each_t_impl(
        t, f, std::make_index_sequence<std::tuple_size<Tuple>::value>{});
}
#endif
