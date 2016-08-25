#include "loop_executor.h"
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <iostream>

using namespace std;


namespace qflow{

enum class state
{
    AVAILABLE = 0,
    LOCKED = 1,
    SLEEPING = 2
};

class loop_executor::loop_executor_private
{
public:
    queue<task>* _head;
    queue<task> _L1;
    queue<task> _L2;
    std::thread _mainThread;
    bool _finished = false;
    std::condition_variable cv;
    mutex m;
    std::atomic<state> _state;
    loop_executor_private()
    {
        _state = state::AVAILABLE;
        _head = &_L1;
        _mainThread = std::thread(&loop_executor_private::main, this);
    }
    ~loop_executor_private()
    {
        _state = state::AVAILABLE;
        _finished = true;
        cv.notify_one();
        _mainThread.join();
    }
    void main()
    {
        queue<task>* other = &_L2;
        while(!_finished)
        {
            state expected = state::AVAILABLE;
            if(_state.compare_exchange_weak(expected, state::LOCKED))
            {
                std::swap(_head, other);
                if(other->empty())
                {
                    std::unique_lock<std::mutex> lk(m);
                    _state = state::SLEEPING;
                    cv.wait(lk);
                }
                else
                {
                    _state = state::AVAILABLE;
                    while(!other->empty())
                    {
                        other->front()();
                        other->pop();
                    }
                }
            }
        }
    }
};
loop_executor::loop_executor() : d_ptr(new loop_executor_private())
{

}
loop_executor::~loop_executor()
{

}
loop_executor::loop_executor(const loop_executor& other) : d_ptr(other.d_ptr)
{

}
void loop_executor::enqueueTask(task t)
{
    bool cont = true;
    state expected = state::AVAILABLE;
    bool wake = false;
    while(cont)
    {
        if(d_ptr->_state.compare_exchange_weak(expected, state::LOCKED))
        {
            if(expected == state::SLEEPING) wake = true;
            cont = false;
        }
        else if(expected == state::LOCKED) expected = state::AVAILABLE;

    }
    d_ptr->_head->push(t);
    if(wake)
    {
        std::unique_lock<std::mutex> lk(d_ptr->m);
        d_ptr->cv.notify_one();
    }
    d_ptr->_state = state::AVAILABLE;
}
}
