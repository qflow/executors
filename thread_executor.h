#include "executor_traits.h"
#include "future.h"

namespace QFlow{
class thread_executor
{
public:
    using task = std::function<void()>;
    void enqueueTask(task t);
};
}

template<>
class executor_traits<QFlow::thread_executor>
{
public:
    using executor_type = QFlow::thread_executor;
    using execution_category = parallel_executor_tag;
    template<class T>
    using future = QFlow::FutureBase<T>;
    template<class T>
    using promise_type = QFlow::Promise<T>;
    template<class Function>
    static future<std::result_of_t<Function()>> async_execute(QFlow::thread_executor& ex, Function&& f)
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
    }
};
