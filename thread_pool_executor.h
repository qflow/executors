#ifndef THREAD_POOL_EXECUTOR_H
#define THREAD_POOL_EXECUTOR_H
#include "for_each.h"
#include "then_execute.h"
#include "standard_executor_traits.h"
#include "future.h"
#include "util/tuple_v.h"
#include "util/for_each_t.h"
#include "util/function_traits.h"

namespace qflow{
using task = std::function<void()>;
class thread_pool_executor_private;
class thread_pool_executor
{
    friend class thread_pool_thread;
public:
    EXECUTORS_EXPORT thread_pool_executor(size_t num_threads = 10);
    ~thread_pool_executor();
    thread_pool_executor(const thread_pool_executor& other);
    void enqueueTask(task t);
private:
    const std::shared_ptr<thread_pool_executor_private> d_ptr;
};
}

template<>
class executor_traits<qflow::thread_pool_executor>
{
public:
    using executor_type = qflow::thread_pool_executor;
    using execution_category = parallel_executor_tag;
    template<class T>
    using future_type = qflow::FutureBase<T>;
    template<class T>
    using promise_type = qflow::Promise<T>;
    template<class T>
    using container = std::vector<T>;

    template<class Function>
    static future_type<std::result_of_t<Function()>> async_execute(qflow::thread_pool_executor& ex, Function&& f)
    {
        using R = std::result_of_t<Function()>;
        using P = std::shared_ptr<promise_type<R>>;
        P promise = std::make_shared<promise_type<R>>();
        auto fut = promise->get_future();
        ex.enqueueTask(std::bind(Call<R, Function, P>::call, f, promise));
        return fut;
    }
    template<class Function, class Future>
    static future_type<result_of_friendly_t<Function, get_template_type_t<Future>>> then_execute(executor_type& ex, Function&& f, Future&& fut)
    {
        using T = get_template_type_t<Future>;
        using R = result_of_friendly_t<Function, T>;
        return then_execute_impl<executor_type, future_type, promise_type, Function, Future, T, R>::exec(ex, std::forward<Function>(f), fut);
    }

    template<typename... ChildFutures>
    using future_tuple = decltype(make_empty_tuple_v<get_template_type_t<ChildFutures>...>());



    template<typename when_future_inner_type, typename when_future_type, typename idx_type, typename counter_type, typename promise_type, typename resultTuple_type>
    struct handle_call
    {
        void operator()(when_future_type&& future, idx_type idx, counter_type counter, promise_type promise, resultTuple_type resultTuple)
        {
            future.then([idx, counter, promise, resultTuple](auto val){
                std::get<idx.value>(*resultTuple.get()) = val;
                size_t c = counter->fetch_sub(1);
                if(c == 1) promise->set_value(*resultTuple.get());
            });
        }
    };
    template<typename when_future_type, typename idx_type, typename counter_type, typename promise_type, typename resultTuple_type>
    struct handle_call<void, when_future_type, idx_type, counter_type, promise_type, resultTuple_type>
    {
        void operator()(when_future_type&& future, idx_type idx, counter_type counter, promise_type promise, resultTuple_type resultTuple)
        {
            future.then([idx, counter, promise, resultTuple](){
                size_t c = counter->fetch_sub(1);
                if(c == 1) promise->set_value(*resultTuple.get());
            });
        }
    };

    template<class... Futures>
    static future_type<decltype(make_empty_tuple_v<get_template_type_t<Futures>...>())> when_all(executor_type&, Futures&&... futures)
    {
        using result_type = future_tuple<Futures...>;
        using P = std::shared_ptr<promise_type<result_type>>;
        P promise = std::make_shared<promise_type<result_type>>();
        future_type<result_type> future = promise->get_future();
        auto futuresTuple = std::forward_as_tuple(futures...);
        std::shared_ptr<result_type> resultTuple = std::make_shared<result_type>();
        std::shared_ptr<std::atomic<size_t>> counter = std::make_shared<std::atomic<size_t>>(sizeof...(futures));

        for_each_t(futuresTuple, [counter, promise, resultTuple](auto idx, auto&& future){
            handle_call<get_template_type_t<decltype(future)>, decltype(future), decltype(idx),
                    std::shared_ptr<std::atomic<size_t>>, P, std::shared_ptr<result_type>>()(future, idx, counter, promise, resultTuple);
            return 0;
        });
        return future;
    }

    template<typename Function, typename Range>
    static future_type<container<function_result_type<Function>>> for_each(executor_type& ex, Function&& func, Range& range)
    {
        using result_type = function_result_type<Function>;
        return for_each_impl<Function, Range, executor_type,
                promise_type, future_type, container, result_type>::for_each(ex, std::forward<Function>(func), range);
    }

};
#endif
