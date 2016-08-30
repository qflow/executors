#ifndef LOOP_EXECUTOR_H
#define LOOP_EXECUTOR_H
#include "for_each.h"
#include "then_execute.h"
#include "when_all.h"
#include "async_execute.h"
#include "standard_executor_traits.h"
#include "future.h"

namespace qflow{

class loop_executor
{
public:
    using task = std::function<void()>;
    EXECUTORS_EXPORT loop_executor();
    ~loop_executor();
    loop_executor(const loop_executor& other);
    void enqueueTask(task t);
private:
    class loop_executor_private;
    const std::shared_ptr<loop_executor_private> d_ptr;
};
}

template<>
class executor_traits<qflow::loop_executor>
{
public:
    using executor_type = qflow::loop_executor;
    using execution_category = parallel_executor_tag;
    template<class T>
    using future_type = qflow::future<T>;
    template<class T>
    using promise_type = qflow::promise<T>;
    template<class T>
    using container = std::vector<T>;

    template<class Function>
    static future_type<std::result_of_t<Function()>> async_execute(executor_type& ex, Function&& f)
    {
        return qflow::async_execute_impl<executor_type, promise_type, future_type, Function>()(ex, std::forward<Function>(f));
    }
    template<class Function, class Future>
    static future_type<result_of_friendly_t<Function, get_template_type_t<Future>>> then_execute(executor_type& ex, Function&& f, Future&& fut)
    {
        return qflow::then_execute_impl<executor_type, future_type, promise_type, Function, Future>()(ex, std::forward<Function>(f), fut);
    }
    template<class... Futures>
    static future_type<decltype(make_empty_tuple_v<get_template_type_t<Futures>...>())> when_all(executor_type& ex, Futures&&... futures)
    {
        return qflow::when_all_impl<executor_type, promise_type, future_type>()(ex, futures...);
    }
    template<typename Function, typename Range>
    static auto for_each(executor_type& ex, Function&& func, Range& range)
    {
        return qflow::for_each_impl<Function, Range, executor_type, promise_type, future_type, container>()(ex, std::forward<Function>(func), range);
    }
};
#endif
