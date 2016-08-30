#include "future.h"

namespace qflow{
promise<void>::promise() : d_ptr(new promise_private<void>())
{

}
promise<void>::promise(promise<void>&& other) : d_ptr(std::move(other.d_ptr))
{

}
void promise<void>::set_value()
{
    d_ptr->_internal.set_value();
    d_ptr->_future->set_value();
}
future<void> promise<void>::get_future()
{
    std::future<void> stdFuture = d_ptr->_internal.get_future();
    future<void> ret(std::move(stdFuture));
    d_ptr->_future = ret.d_ptr;
    return ret;
}
}
