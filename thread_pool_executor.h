#ifndef THREAD_POOL_EXECUTOR_H
#define THREAD_POOL_EXECUTOR_H
#include "standard_executor_traits.h"
#include "future.h"
#include "util/tuple_v.h"

namespace QFlow{
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
class executor_traits<QFlow::thread_pool_executor>
{
public:
    using executor_type = QFlow::thread_pool_executor;
    using execution_category = parallel_executor_tag;
    template<class T>
    using future_type = QFlow::FutureBase<T>;
    template<class T>
    using promise_type = QFlow::Promise<T>;
    template<class Function>
    static future_type<std::result_of_t<Function()>> async_execute(QFlow::thread_pool_executor& ex, Function&& f)
    {
        using R = std::result_of_t<Function()>;
        using P = std::shared_ptr<promise_type<R>>;
        P p = std::make_shared<promise_type<R>>();
        auto fut = p->get_future();
        std::function<void()> task = Call<R, Function, P>::get_task(f, p);
        ex.enqueueTask(task);
        return fut;
    }
    template<class Function, class T>
    static future_type<result_of_friendly_t<Function, T>> then_execute(executor_type& ex, Function&& f, future_type<T>&& fut)
    {
        using R = result_of_friendly_t<Function, T>;
        using P = std::shared_ptr<promise_type<R>>;
        P promise = std::make_shared<promise_type<R>>();
        future_type<R> resF = promise->get_future();
        auto task = Call2<R, Function, P, T, executor_type>::get_task(f, promise, ex);
        fut.then(task);
        return resF;
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

        apply(futuresTuple, [counter, promise, resultTuple](auto idx, auto&& future){
            handle_call<get_template_type_t<decltype(future)>, decltype(future), decltype(idx),
                    std::shared_ptr<std::atomic<size_t>>, P, std::shared_ptr<result_type>>()(future, idx, counter, promise, resultTuple);
            return 0;
        });
        return future;
    }

};
#endif
