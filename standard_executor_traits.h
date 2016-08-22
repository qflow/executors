#ifndef STANDARD_EXECUTOR_TRAITS_H
#define STANDARD_EXECUTOR_TRAITS_H
#include "executor_traits.h"
#include <type_traits>

template<typename R, typename F, typename P>
struct Call
{
static std::function<void()> get_task(F f, P p)
{
    std::function<void()> task = [p, f](){
        auto res = f();
        p->set_value(res);
    };
    return task;
}
};
template<typename F, typename P>
struct Call<void, F, P>
{
static std::function<void()> get_task(F f, P p)
{
    std::function<void()> task = [p, f](){
        f();
        p->set_value();
    };
    return task;
}
};

template<typename R, typename F, typename P, typename T, typename executor_type>
struct Call2
{
    static std::function<void(T)> get_task(F f, P promise, executor_type& ex)
    {
        std::function<void(T)> task = [&ex, f, promise](T value){
            auto fut2 = executor_traits<executor_type>::async_execute(ex, std::bind(f, value));
            fut2.then([promise](R value2){
                promise->set_value(value2);
            });
        };
        return task;
    }
};
template<typename R, typename F, typename P, typename executor_type>
struct Call2<R, F, P, void, executor_type>
{
    static std::function<void()> get_task(F f, P promise, executor_type& ex)
    {
        std::function<void()> task = [&ex, f, promise](){
            auto fut2 = executor_traits<executor_type>::async_execute(ex, f);
            fut2.then([promise](R value2){
                promise->set_value(value2);
            });
        };
        return task;
    }
};
template<typename F, typename P, typename T, typename executor_type>
struct Call2<void, F, P, T, executor_type>
{
    static std::function<void(T)> get_task(F f, P promise, executor_type& ex)
    {
        std::function<void(T)> task = [&ex, f, promise](T value){
            auto fut2 = executor_traits<executor_type>::async_execute(ex, std::bind(f, value));
            fut2.then([promise](){
                promise->set_value();
            });
        };
        return task;
    }
};
template<typename F, typename P, typename executor_type>
struct Call2<void, F, P, void, executor_type>
{
    static std::function<void()> get_task(F f, P promise, executor_type& ex)
    {
        std::function<void()> task = [&ex, f, promise](){
            auto fut2 = executor_traits<executor_type>::async_execute(ex, f);
            fut2.then([promise](){
                promise->set_value();
            });
        };
        return task;
    }
};
template< class F, class T>
struct result_of_friendly
{
    using type = std::result_of_t<F(T)>;
};

template< class F>
struct result_of_friendly<F, void>
{
    using type = std::result_of_t<F()>;
};
template< class F, class T>
using result_of_friendly_t = typename result_of_friendly<F, T>::type;

template<class Function, typename Executor>
typename executor_traits<Executor>::template future_type<std::result_of_t<Function()>> async_execute(Executor& ex, Function&& f)
{
    return executor_traits<Executor>::async_execute(ex, f);
}
template<class Function, template<typename> class Future, class T, class Executor>
typename executor_traits<Executor>::template future_type<result_of_friendly_t<Function, T>> then_execute(Executor& ex, Function&& f, Future<T>&& fut)
{
    return executor_traits<Executor>::then_execute(ex, f, std::forward<Future<T>>(fut));
}

template<typename V, typename Function>
static int for_each_impl(V&& value, Function&& f)
{
    f(std::forward<V>(value));
    return 0;
}
template<typename... Values, typename Function>
static void for_each(Function&& f, Values&&... values)
{
    int arr[sizeof...(values)] = {for_each_impl<Values, Function>(std::forward<Values>(values),
                                                                  std::forward<Function>(f))...};
}
template<typename T, size_t size>
constexpr bool isSize()
{
    return std::tuple_size<T>::value == size;
}


template <class F, size_t... Is>
constexpr auto index_apply_impl(F f,
                                std::index_sequence<Is...>) {
    return std::make_tuple(f(std::integral_constant<size_t, Is>())...);
}

template <size_t N, class F>
constexpr auto index_apply(F f) {
    return index_apply_impl(f, std::make_index_sequence<N>{});
}

template <class Tuple, class F, size_t... Is>
constexpr auto apply_impl(Tuple t, F f,
                          std::index_sequence<Is...>) {
    return std::make_tuple(f(std::integral_constant<size_t, Is>(), std::get<Is>(t))...);
}

template <class Tuple, class F>
constexpr auto apply(Tuple t, F f) {
    return apply_impl(
        t, f, std::make_index_sequence<std::tuple_size<Tuple>::value>{});
}



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
constexpr decltype(auto) make_empty_tuple_v()
{
    return std::tuple_cat(tuple_v<T>().create()...);
}
template<typename... T>
constexpr decltype(auto) make_tuple_v(T&&... args)
{
    return std::tuple_cat(tuple_v<T>().create(std::forward<T>(args))...);
}



#endif
