#ifndef FOR_EACH_T_H
#define FOR_EACH_T_H

#include "function_traits.h"
#include <tuple>

template <class Tuple, class F>
struct for_each_t_impl
{
    template<size_t... Is>
    auto execute(Tuple t, F&& f,
                           std::index_sequence<Is...>) {
        return std::make_tuple(f(std::integral_constant<size_t, Is>(), std::get<Is>(t))...);
    }
};


template <class Tuple, class F>
auto for_each_t(Tuple t, F&& f) {
    //using result_type = typename function_traits<F>::result_type;
    for_each_t_impl<Tuple, F> impl;
    return impl.execute(t, std::forward<F>(f), std::make_index_sequence<std::tuple_size<Tuple>::value>{});
}
#endif
