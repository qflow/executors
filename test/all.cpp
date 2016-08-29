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


    for(int i=0;i<1000;i++)
    {
        auto f = async_execute(tpe, [i](){
            return i;
        });
        //futures.push_back(std::move(f));

        auto ff = async_execute(tpe, [i](){
            return 0;
        });
        //futures.push_back(std::move(ff));
        auto fff = then_execute(tpe, [i](int g){
            return 1;
        }, ff);
        fff.wait();
        int t=0;
    }
}