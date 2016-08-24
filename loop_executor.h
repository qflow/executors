#ifndef LOOP_EXECUTOR_H
#define LOOP_EXECUTOR_H
#include "executors_global.h"
#include "standard_executor_traits.h"
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
    using future_type = QFlow::FutureBase<T>;
    template<class T>
    using promise_type = QFlow::Promise<T>;
    template<class Function>
    static future_type<std::result_of_t<Function()>> async_execute(executor_type& ex, Function&& f)
    {
        using R = std::result_of_t<Function()>;
        using P = std::shared_ptr<promise_type<R>>;
        P p = std::make_shared<promise_type<R>>();
        auto fut = p->get_future();
        ex.enqueueTask(std::bind(Call<R, Function, P>::call, f, p));
        return fut;
    }
    template<class Function, class T>
    static future_type<result_of_friendly_t<Function, T>> then_execute(executor_type& ex, Function&& f, future_type<T>& fut)
    {
        using R = result_of_friendly_t<Function, T>;
        using P = std::shared_ptr<promise_type<R>>;
        P promise = std::make_shared<promise_type<R>>();
        future_type<R> resF = promise->get_future();
        auto task = Call2<R, Function, P, T, executor_type>::get_task(f, promise, ex);
        fut.then(task);
        return resF;
    }
};
#endif
