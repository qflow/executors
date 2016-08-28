#ifndef STANDARD_EXECUTOR_TRAITS_H
#define STANDARD_EXECUTOR_TRAITS_H
#include "executor_traits.h"

template<class Function, class Executor>
auto async_execute(Executor& ex, Function&& f)
{
    return executor_traits<Executor>::async_execute(ex, f);
}
template<class Function, class Future, class Executor>
auto then_execute(Executor& ex, Function&& f, Future&& fut)
{
    return executor_traits<Executor>::then_execute(ex, f, fut);
}
template<class executor_type, class... Futures>
auto when_all(executor_type& ex, Futures&&... futures)
{
    return executor_traits<executor_type>::when_all(ex, futures...);
}
template<class executor_type, class Function, class Range>
auto for_each(executor_type& ex, Function&& func, Range& range)
{
    return executor_traits<executor_type>::for_each(ex, func, range);
}
#endif
