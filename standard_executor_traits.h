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


#endif
