#ifndef THEN_EXECUTE_H
#define THEN_EXECUTE_H
#include "util/function_traits.h"
#include "util/result_of_friendly.h"
#include "executor_traits.h"
#include <memory>
#include <functional>
#include <atomic>

namespace qflow{

template<typename executor_type, template<typename> class future_type, template<typename> class promise_type,
         class Function, class Future, class T, class R>
struct then_execute_impl_s
{
future_type<R> operator ()(executor_type& ex, Function&& f, Future&& fut)
{
    using P = std::shared_ptr<promise_type<R>>;
    P promise = std::make_shared<promise_type<R>>();
    future_type<R> resF = promise->get_future();
    fut.then([&ex, f, promise](auto value){
        std::function<R()> task = std::bind(f, value);
        future_type<R> fut2 = executor_traits<executor_type>::async_execute(ex, task);
        fut2.then([promise](R value2){
            promise->set_value(value2);
        });
    });
    return resF;
}
};

template<typename executor_type, template<typename> class future_type, template<typename> class promise_type,
         class Function, class Future, class T>
struct then_execute_impl_s<executor_type, future_type, promise_type, Function, Future, T, void>
{
future_type<void> operator ()(executor_type& ex, Function&& f, Future&& fut)
{
    using P = std::shared_ptr<promise_type<void>>;
    P promise = std::make_shared<promise_type<void>>();
    future_type<void> resF = promise->get_future();
    fut.then([&ex, f, promise](auto value){
        std::function<void()> task = std::bind(f, value);
        auto fut2 = executor_traits<executor_type>::async_execute(ex, task);
        fut2.then([promise](){
            promise->set_value();
        });
    });
    return resF;
}
};

template<typename executor_type, template<typename> class future_type, template<typename> class promise_type,
         class Function, class Future, class R>
struct then_execute_impl_s<executor_type, future_type, promise_type, Function, Future, void, R>
{
future_type<R> operator ()(executor_type& ex, Function&& f, Future&& fut)
{
    using P = std::shared_ptr<promise_type<R>>;
    P promise = std::make_shared<promise_type<R>>();
    future_type<R> resF = promise->get_future();
    fut.then([&ex, f, promise](){
        auto fut2 = executor_traits<executor_type>::async_execute(ex, f);
        fut2.then([promise](R value2){
            promise->set_value(value2);
        });
    });
    return resF;
}
};

template<typename executor_type, template<typename> class future_type, template<typename> class promise_type,
         class Function, class Future>
struct then_execute_impl_s<executor_type, future_type, promise_type, Function, Future, void, void>
{
future_type<void> operator ()(executor_type& ex, Function&& f, Future&& fut)
{
    using P = std::shared_ptr<promise_type<void>>;
    P promise = std::make_shared<promise_type<void>>();
    future_type<void> resF = promise->get_future();
    fut.then([&ex, f, promise](){
        auto fut2 = executor_traits<executor_type>::async_execute(ex, f);
        fut2.then([promise](){
            promise->set_value();
        });
    });
    return resF;
}
};

template<typename executor_type, template<typename> class future_type, template<typename> class promise_type,
         class Function, class Future>
using then_execute_impl = then_execute_impl_s<executor_type, future_type, promise_type, Function, Future,
        get_template_type_t<Future>, result_of_friendly_t<Function, get_template_type_t<Future>>>;
}
#endif
