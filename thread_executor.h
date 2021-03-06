#include "executor_traits.h"
#include "future.h"

namespace qflow{
class thread_executor
{
public:
    using task = std::function<void()>;
    void enqueueTask(task t);
};
}

template<>
class executor_traits<qflow::thread_executor>
{
public:
    using executor_type = qflow::thread_executor;
    using execution_category = parallel_executor_tag;
    template<class T>
    using future_type = qflow::future<T>;
    template<class T>
    using promise_type = qflow::promise<T>;
    template<class Function>
    static future_type<std::result_of_t<Function()>> async_execute(qflow::thread_executor& ex, Function&& f)
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
    template<class Function, class T, class R = std::result_of_t<Function(T)>>
    static future_type<R> then_execute(executor_type& ex, Function&& f, future_type<T>& fut)
    {
        auto promise = std::make_shared<promise_type<R>>();
        std::promise<int> p;
        future_type<R> resF = promise->get_future();
        fut.then([&ex, f, promise](T value){
            future_type<R> fut2 = executor_traits<executor_type>::async_execute(ex, std::bind(f, value));
            fut2.then([promise](R value2){
                promise->set_value(value2);
            });
        });
        return resF;
    }
};
