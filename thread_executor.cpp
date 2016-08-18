#include "thread_executor.h"
#include <thread>

namespace QFlow{
void thread_executor::enqueueTask(task t)
{
    std::async(t);
}
}
