#include "thread_pool_executor.h"
#include <thread>
#include <queue>
#include <vector>
#include <mutex>

namespace QFlow{

class thread_pool_thread
{
public:
    thread_pool_thread() : d_ptr(new thread_pool_thread_private())
    {
    }
    ~thread_pool_thread()
    {
    }
    thread_pool_thread(const thread_pool_thread& other) : d_ptr(other.d_ptr)
    {

    }
    void processTask(thread_pool_executor::task t)
    {
        d_ptr->processTask(t);
    }


private:
    class thread_pool_thread_private
    {
    public:
        std::mutex m;
        std::thread _mainThread;
        bool _finished = false;
        std::condition_variable cv;
        thread_pool_executor::task _task;
        thread_pool_thread_private()
        {
            _mainThread = std::thread(&thread_pool_thread_private::main, this);
        }
        ~thread_pool_thread_private()
        {
            _finished = true;
            cv.notify_one();
            _mainThread.join();
        }

        void main()
        {
            while(!_finished)
            {
                if(_task)
                {
                    _task();
                    _task = nullptr;
                }
                std::unique_lock<std::mutex> lk(m);
                cv.wait(lk);
            }
        }
        void processTask(thread_pool_executor::task t)
        {
            _task = t;
            cv.notify_one();
        }
    };
    std::shared_ptr<thread_pool_thread_private> d_ptr;
};



class thread_pool_executor::thread_pool_executor_private
{
public:
    std::atomic_flag _taskQueuelock = ATOMIC_FLAG_INIT;
    std::atomic_flag _threadQueuelock = ATOMIC_FLAG_INIT;
    std::queue<task> _taskQueue;
    std::queue<task> _localTaskQueue;
    std::queue<thread_pool_thread> _threadQueue;
    std::vector<thread_pool_thread> _pool;
    std::thread _mainThread;
    bool _finished = false;
    std::condition_variable cv;
    thread_pool_executor_private()
    {
        _mainThread = std::thread(&thread_pool_executor_private::main, this);
    }

    ~thread_pool_executor_private()
    {
        _finished = true;
        cv.notify_one();
        _mainThread.join();
    }

    void main()
    {
        while(!_finished)
        {
            _taskQueuelock.test_and_set(std::memory_order_acquire);
            while(!_taskQueue.empty())
            {
                _localTaskQueue.push(_taskQueue.front());
                _taskQueue.pop();
            }
            _taskQueuelock.clear();
            std::vector<task> tasks;
            std::vector<thread_pool_thread> threads;
            _threadQueuelock.test_and_set(std::memory_order_acquire);
            while(!_localTaskQueue.empty() && !_threadQueue.empty())
            {
                tasks.push_back(_localTaskQueue.front());
                _localTaskQueue.pop();
                threads.push_back(_threadQueue.front());
                _threadQueue.pop();
            }
            _threadQueuelock.clear();
            for(size_t i=0;i<tasks.size();i++)
            {
                threads[i].processTask(tasks[i]);
            }


        }
    }
};
thread_pool_executor::thread_pool_executor(size_t num_threads) : d_ptr(new thread_pool_executor_private())
{
    d_ptr->_pool.resize(num_threads);
    for(thread_pool_thread t: d_ptr->_pool) d_ptr->_threadQueue.push(t);
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
