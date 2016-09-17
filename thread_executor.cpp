#include "thread_executor.h"
#include <thread>

namespace qflow{
void thread_executor::enqueueTask(task t)
{
    std::async(t);
}
}
