#include "thread_pool_executor.h"
#include "thread_pool_executor_p.h"
#include <thread>
#include <queue>
#include <vector>
#include <mutex>
#include <iostream>
#include <sstream>

namespace QFlow{
thread_pool_executor_private::thread_pool_executor_private()
{
    _mainThread = std::thread(&thread_pool_executor_private::main, this);
}
thread_pool_executor_private::~thread_pool_executor_private()
{
    _finished = true;
    cv.notify_one();
    _mainThread.join();
}
void thread_pool_executor_private::main()
{
    while(!_finished)
    {
        if(!_taskQueuelock.test_and_set(std::memory_order_acquire))
        {
            while(!_taskQueue.empty())
            {
                _localTaskQueue.push(_taskQueue.front());
                _taskQueue.pop();
            }
            _taskQueuelock.clear();
        }
        std::vector<task> tasks;
        std::vector<loop_executor> threads;
        if(!_threadQueuelock.test_and_set(std::memory_order_acquire))
        {
            while(!_localTaskQueue.empty() && !_threadQueue.empty())
            {
                tasks.push_back(_localTaskQueue.front());
                _localTaskQueue.pop();
                threads.push_back(_threadQueue.front());
                _threadQueue.pop();
            }
            _threadQueuelock.clear();
        }
        for(size_t i=0;i<tasks.size();i++)
        {
            loop_executor thread = threads[i];
            auto f = executor_traits<loop_executor>::async_execute(thread, tasks[i]);
            auto ff = f.then([this, thread](){
                while (_threadQueuelock.test_and_set(std::memory_order_acquire));
                _threadQueue.push(thread);
                _threadQueuelock.clear();
            });
            ff.wait();
        }


    }
}
thread_pool_executor::thread_pool_executor(size_t num_threads) : d_ptr(new thread_pool_executor_private())
{
    for(size_t i=0;i<num_threads;i++)
    {
        loop_executor thread;
        d_ptr->_pool.push_back(thread);
        d_ptr->_threadQueue.push(thread);
    }
}
thread_pool_executor::~thread_pool_executor()
{

}
thread_pool_executor::thread_pool_executor(const thread_pool_executor& other) : d_ptr(other.d_ptr)
{

}

void thread_pool_executor::enqueueTask(task t)
{
    while (d_ptr->_taskQueuelock.test_and_set(std::memory_order_acquire));
    d_ptr->_taskQueue.push(t);
    d_ptr->_taskQueuelock.clear();
}
}
