#include "future.h"
#include "executor_traits.h"
#include "loop_executor.h"
#include "thread_executor.h"
#include "thread_pool_executor.h"
#include <iostream>
#include <thread>
#include <vector>
#include <cassert>

int main()
{
    qflow::thread_pool_executor tpe(2);
    qflow::loop_executor executor;
    std::vector<qflow::FutureBase<std::tuple<int, bool>>> futures;

    std::vector<int> range = {0,1,2,3};

    auto future = executor_traits<qflow::thread_pool_executor>::for_each(tpe, [](int val){
        return val*2;
    }, range);
    auto res = future.get();
    assert(res.size() == range.size());
    for(size_t i=0;i<res.size();i++)
    {
        assert(res[i] == i*2);
    }

    std::vector<qflow::FutureBase<int>> vec;
    for(int i=0;i<1000;i++)
    {
        auto f = async_execute(tpe, [i](){
            return i;
        });
        vec.push_back(std::move(f));
    }
    for(size_t i=0;i<1000;i++)
    {
        auto f = std::move(vec[i]);
        int res = f.get();
        assert(res == i);
    }

    for(int i=0;i<1000;i++)
    {
        auto f = async_execute(tpe, [i](){
            return 1;
        });
        auto ff = async_execute(tpe, [i](){
            return 2;
        });
        auto fff = when_all(tpe, f, ff);
        auto ffff = then_execute(tpe, [](auto arg){
            return std::get<0>(arg) + std::get<1>(arg);
        }, fff);
        int sum = ffff.get();
        assert(sum == 3);
    }
}
