#ifndef ASYNC_EXECUTE_H
#define ASYNC_EXECUTE_H

#include <type_traits>
#include <memory>

namespace qflow {


template<typename R, typename Function, typename Promise_ptr>
struct Call
{
    static void call(Function f, Promise_ptr p)
    {
        auto res = f();
        p->set_value(res);
    }
};
template<typename Function, typename Promise_ptr>
struct Call<void, Function, Promise_ptr>
{
    static void call(Function f, Promise_ptr p)
    {
        f();
        p->set_value();
    }
};

template<typename executor_type, template<typename> class promise_type, template<typename> class future_type, class Function>
struct async_execute_impl
{
future_type<std::result_of_t<Function()>> operator ()(executor_type& ex, Function&& f)
{
    using R = std::result_of_t<Function()>;
    using P = std::shared_ptr<promise_type<R>>;
    P promise = std::make_shared<promise_type<R>>();
    auto fut = promise->get_future();
    std::function<void()> task = std::bind(Call<R, Function, P>::call, f, promise);
    ex.enqueueTask(task);
    return fut;
}
};

}
#endif
