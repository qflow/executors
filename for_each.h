#ifndef FOR_EACH_H
#define FOR_EACH_H
#include "util/function_traits.h"
#include "executor_traits.h"
#include "type_traits"
#include <memory>
#include <functional>
#include <atomic>

namespace qflow{

template<typename Function, typename Range, typename executor_type, template <typename> class promise_type,
         template <typename> class future_type, template <typename> class container,
         typename result_type>
struct for_each_impl_s
{
    future_type<container<result_type>> operator()(executor_type& ex, Function&& func, Range& range)
    {
        std::shared_ptr<promise_type<container<result_type>>> promise =
                std::make_shared<promise_type<container<result_type>>>();
        future_type<container<result_type>> res_future = promise->get_future();
        std::shared_ptr<std::atomic<int>> counter = std::make_shared<std::atomic<int>>(0);
        std::shared_ptr<container<result_type>> result_values = std::make_shared<container<result_type>>();
        std::shared_ptr<std::atomic_flag> resultLock = std::make_shared<std::atomic_flag>();
        result_type c2 = 0;
        for(auto value: range)
        {
            executor_traits<executor_type>::async_execute(ex, [c2, promise, func, value, counter, resultLock, result_values](){
                result_type res = func(value);
                int c = counter->fetch_add(1)+1;
                while (resultLock->test_and_set(std::memory_order_acquire));
                if(result_values->size() <= c2) result_values->resize(c2 + 1);
                result_values->at(c2) = res;
                resultLock->clear();
                if(c==0)
                {
                    promise->set_value(*result_values.get());
                }
            });
            c2++;
        }
        int c = counter->fetch_sub(c2)-c2;
        if(c ==0)
        {
            promise->set_value(*result_values.get());
        }
        return res_future;
    }
};

template<typename Function, typename Range, typename executor_type, template <typename> class promise_type,
         template <typename> class future_type, template <typename> class container>
struct for_each_impl_s<Function, Range, executor_type, promise_type, future_type, container, void>
{
    future_type<void> operator ()(executor_type& ex, Function&& func, Range& range)
    {
        std::shared_ptr<promise_type<void>> promise = std::make_shared<promise_type<void>>();
        future_type<void> res_future = promise->get_future();
        std::shared_ptr<std::atomic<int>> counter = std::make_shared<std::atomic<int>>(0);
        int c2 = 0;
        for(auto idx: range)
        {
            c2++;
            executor_traits<executor_type>::async_execute(ex, [idx, func, counter, promise](){
                func(idx);
                int c = counter->fetch_add(1)+1;
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

template<typename Function, typename Range, typename executor_type, template <typename> class promise_type,
         template <typename> class future_type, template <typename> class container>
using for_each_impl = for_each_impl_s<Function, Range, executor_type, promise_type, future_type, container, function_result_type<Function>>;

}
#endif
