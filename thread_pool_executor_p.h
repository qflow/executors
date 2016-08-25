#ifndef THREAD_POOL_EXECUTOR_P_H
#define THREAD_POOL_EXECUTOR_P_H
#include "executor_traits.h"
#include "future.h"
#include "loop_executor.h"
#include <queue>

namespace qflow{
using task = std::function<void()>;

class thread_pool_executor_private
{
public:
    std::atomic_flag _taskQueuelock = ATOMIC_FLAG_INIT;
    std::atomic_flag _threadQueuelock = ATOMIC_FLAG_INIT;
    std::queue<task> _taskQueue;
    std::queue<task> _localTaskQueue;
    std::queue<loop_executor> _threadQueue;
    std::vector<loop_executor> _pool;
    std::thread _mainThread;
    bool _finished = false;
    std::condition_variable cv;
    thread_pool_executor_private();

    ~thread_pool_executor_private();
    void main();
};
}
#endif
