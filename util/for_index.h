#ifndef FOR_INDEX_H
#define FOR_INDEX_H

#include <tuple>

template <class F>
struct for_index_impl
{
    template<size_t... Is>
    auto execute(F&& f, std::index_sequence<Is...>) {
        return std::make_tuple(f(std::integral_constant<size_t, Is>())...);
    }
};


template <size_t N, typename F>
auto for_index(F&& f) {
    //using result_type = typename function_traits<F>::result_type;
    for_index_impl<F> impl;
    return impl.execute(std::forward<F>(f), std::make_index_sequence<N>{});
}
#endif
