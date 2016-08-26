#ifndef WHEN_ALL_H
#define WHEN_ALL_H

#include "util/template_type.h"
#include "util/tuple_v.h"
#include "util/for_each_t.h"
#include <memory>

namespace qflow{

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

template<typename... ChildFutures>
    using future_tuple = decltype(make_empty_tuple_v<get_template_type_t<ChildFutures>...>());    
    
template<typename executor_type, template<typename> class promise_type, template<typename> class future_type>
struct when_all_impl
{
    template<class... Futures>
    future_type<decltype(make_empty_tuple_v<get_template_type_t<Futures>...>())> operator()(executor_type&, Futures&&... futures)
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
};
}
#endif
