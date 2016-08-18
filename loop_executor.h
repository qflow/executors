#ifndef LOOP_EXECUTOR_H
#define LOOP_EXECUTOR_H
#include "executors_global.h"
#include "executor_traits.h"
#include "future.h"
#include <functional>
#include <future>

namespace QFlow{

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
class executor_traits<QFlow::loop_executor>
{
public:
    using executor_type = QFlow::loop_executor;
    using execution_category = parallel_executor_tag;
    template<class T>
    using future = QFlow::FutureBase<T>;
    template<class T>
    using promise_type = QFlow::Promise<T>;
    template<class Function>
    static future<std::result_of_t<Function()>> async_execute(QFlow::loop_executor& ex, Function&& f);
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

template<class Function>
executor_traits<QFlow::loop_executor>::future<std::result_of_t<Function()>> executor_traits<QFlow::loop_executor>::async_execute(QFlow::loop_executor& ex, Function&& f)
{
        using R = std::result_of_t<Function()>;
        using P = std::shared_ptr<executor_traits<QFlow::loop_executor>::promise_type<R>>;
        P p = std::make_shared<executor_traits<QFlow::loop_executor>::promise_type<R>>();
        auto fut = p->get_future();
        std::function<void()> task = Call<R, Function, P>::get_task(f, p);
        ex.enqueueTask(task);
        return fut;
}
#endif
