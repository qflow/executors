#ifndef THREAD_POOL_EXECUTOR_H
#define THREAD_POOL_EXECUTOR_H
#include "standard_executor_traits.h"
#include "future.h"
#include "util/tuple_v.h"
#include "util/for_each.h"
#include "util/function_traits.h"

namespace NAMESPACE{
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
class executor_traits<NAMESPACE::thread_pool_executor>
{
public:
    using executor_type = QFlow::thread_pool_executor;
    using execution_category = parallel_executor_tag;
    template<class T>
    using future_type = QFlow::FutureBase<T>;
    template<class T>
    using promise_type = QFlow::Promise<T>;
    template<class T>
    using container = std::vector<T>;

    template<class Function>
    static future_type<std::result_of_t<Function()>> async_execute(QFlow::thread_pool_executor& ex, Function&& f)
    {
        using R = std::result_of_t<Function()>;
        using P = std::shared_ptr<promise_type<R>>;
        P p = std::make_shared<promise_type<R>>();
        auto fut = p->get_future();
        ex.enqueueTask(std::bind(Call<R, Function, P>::call, f, p));
        return fut;
    }
    template<class Function, class Future>
    static future_type<result_of_friendly_t<Function, get_template_type_t<Future>>> then_execute(executor_type& ex, Function&& f, Future&& fut)
    {
        using T = get_template_type_t<Future>;
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

        for_each_t(futuresTuple, [counter, promise, resultTuple](auto idx, auto&& future){
            handle_call<get_template_type_t<decltype(future)>, decltype(future), decltype(idx),
                    std::shared_ptr<std::atomic<size_t>>, P, std::shared_ptr<result_type>>()(future, idx, counter, promise, resultTuple);
            return 0;
        });
        return future;
    }

    template<typename Function, typename Range>
    static auto for_each(executor_type& ex, Function&& func, Range& range)
    {

        typedef function_traits<Function> traits;
        using arg_type = typename traits::template arg<0>::type;
        using result_type = typename traits::result_type;
        std::shared_ptr<promise_type<void>> promise = std::make_shared<promise_type<void>>();
        future_type<void> res_future = promise->get_future();
        std::shared_ptr<std::atomic<int>> counter = std::make_shared<std::atomic<int>>(0);
        int c2 = 0;
        for(auto idx: range)
        {
            c2++;
            std::function<result_type()> a = std::bind(func, idx);
            future_type<result_type> future = executor_traits<executor_type>::async_execute(ex, a);
            future.then([counter, promise](auto val){
                int c = (*counter.get())++;
                if(c==0)
                {
                    promise->set_value();
                }
            });
        }
        int c = counter->fetch_sub(c2)-c2;
        if(c ==0)
        {
            promise->set_value();
        }
        return res_future;
    }

};
#endif
