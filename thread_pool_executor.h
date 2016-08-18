#ifndef THREAD_POOL_EXECUTOR_H
#define THREAD_POOL_EXECUTOR_H
#include "executor_traits.h"
#include "future.h"

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
    using future = QFlow::FutureBase<T>;
    template<class T>
    using promise_type = QFlow::Promise<T>;
    template<class Function>
    static future<std::result_of_t<Function()>> async_execute(QFlow::thread_pool_executor& ex, Function&& f)
    {
        auto p = std::make_shared<promise_type<std::result_of_t<Function()>>>();
        auto fut = p->get_future();
        std::function<void()> task = [p, f](){
            auto res = f();
            p->set_value(res);
        };
        ex.enqueueTask(task);
        return fut;
    }
    /*template<class Function, class T, class R = std::result_of_t<Function(T)>>
    static future<R> then_execute(executor_type& ex, Function&& f, future<T>& fut)
    {
        auto promise = std::make_shared<promise_type<R>>();
        std::promise<int> p;
        future<R> resF = promise->get_future();
        fut.then([&ex, f, promise](T value){
            future<R> fut2 = executor_traits<executor_type>::async_execute(ex, std::bind(f, value));
            fut2.then([promise](R value2){
                promise->set_value(value2);
            });
        });
        return resF;
    }*/
};
#endif
