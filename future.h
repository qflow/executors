#ifndef FUTURE_H
#define FUTURE_H

#include "executors_export.h"
#include <functional>
#include <future>
#include <atomic>

namespace qflow{

template<typename T>
class future_private
{
public:
    future_private() : _isReady(false)
    {

    }
    void set_value(T value)
    {
        while (_readyLock.test_and_set(std::memory_order_acquire));
        _value = value;
        _isReady = true;
        if(_onReady)
        {
            _readyLock.clear();
            _onReady(value);
        }
        else _readyLock.clear();
    }

    std::future<T> _stdFuture;
    std::function<void(T)> _onReady;
    bool _isReady;
    std::atomic_flag _readyLock = ATOMIC_FLAG_INIT;
    T _value;
};
template<>
class future_private<void>
{
public:
    future_private() : _isReady(false)
    {

    }
    void set_value()
    {
        while (_readyLock.test_and_set(std::memory_order_acquire));
        _isReady = true;
        if(_onReady)
        {
            _readyLock.clear();
            _onReady();
        }
        else _readyLock.clear();

    }
    std::future<void> _stdFuture;
    std::function<void()> _onReady;
    bool _isReady;
    std::atomic_flag _readyLock = ATOMIC_FLAG_INIT;
};

template<typename T>
class future;

template<>
class future<void>;

template<typename T>
class promise_private
{
public:
    std::promise<T> _internal;
    std::shared_ptr<future_private<T>> _future;
};

template<typename T>
class promise
{
public:
    promise();
    promise(const promise<T>& other) = delete;
    promise(promise<T>&& other);
    future<T> get_future();
    void set_value( const T& value );
private:
    std::unique_ptr<promise_private<T>> d_ptr;
};
template<>
class promise<void>
{
public:
    promise();
    promise(const promise<void>& other) = delete;
    promise(promise<void>&& other);
    future<void> get_future();
    void set_value();
private:
    std::unique_ptr<promise_private<void>> d_ptr;
};

template<typename T>
class future
{
    friend class promise<T>;
    using value_type = T;
public:
    future() : d_ptr(new future_private<T>())
    {

    }
    future(const future& other) = delete;
    future(future&& other) : future()
    {
        d_ptr = std::move(other.d_ptr);
    }

    future(std::future<T>&& other) : future()
    {
        d_ptr->_stdFuture = std::move(other);
    }

    void wait() const
    {
        d_ptr->_stdFuture.wait();
    }
    bool valid() const
    {
        return d_ptr->_stdFuture.valid();
    }
    T value() const
    {
        return d_ptr->_value;
    }
    T get()
    {
        return d_ptr->_stdFuture.get();
    }
    template<typename Func, typename R = std::result_of_t<Func(T)>>
    future<R> then(Func&& func);
protected:
    void set_value(T value)
    {
        d_ptr->set_value(value);
    }


    std::shared_ptr<future_private<T>> d_ptr;
};
template<>
class future<void>
{
    friend class promise<void>;
public:
    future() : d_ptr(new future_private<void>())
    {

    }
    future(std::future<void>&& other) : future()
    {
        d_ptr->_stdFuture = std::move(other);
    }
    future(const future& other) = delete;
    future(future&& other) : future()
    {
        d_ptr = std::move(other.d_ptr);
    }
    void get()
    {
        d_ptr->_stdFuture.get();
    }
    void wait() const
    {
        d_ptr->_stdFuture.wait();
    }
    template<typename Func, typename R = std::result_of_t<Func()>>
    future<R> then(Func&& func);
protected:
    void set_value()
    {
        d_ptr->set_value();

    }
    std::shared_ptr<future_private<void>> d_ptr;
};

template<typename T>
promise<T>::promise() : d_ptr(new promise_private<T>())
{

}
template<typename T>
promise<T>::promise(promise<T>&& other) : d_ptr(std::move(other.d_ptr))
{

}
template<typename T>
future<T> promise<T>::get_future()
{
    std::future<T> stdFuture = d_ptr->_internal.get_future();
    future<T> ret(std::move(stdFuture));
    d_ptr->_future = ret.d_ptr;
    return ret;
}
template<typename T>
void promise<T>::set_value( const T& value )
{
    d_ptr->_internal.set_value(value);
    d_ptr->_future->set_value(value);
}

template<typename R, typename T, typename Func>
void call(std::shared_ptr<promise<R>> promise, Func&& func, T value)
{
    R resVal = func(value);
    promise->set_value(resVal);
}
template<typename R, typename Func>
void call(std::shared_ptr<promise<R>> promise, Func&& func)
{
    R resVal = func();
    promise->set_value(resVal);
}
template<typename T, typename Func>
void call(std::shared_ptr<promise<void>> promise, Func&& func, T value)
{
    func(value);
    promise->set_value();
}
template<typename Func>
void call(std::shared_ptr<promise<void>> promise, Func&& func)
{
    func();
    promise->set_value();
}

template<typename T>
template<typename Func, typename R>
future<R> future<T>::then(Func&& func)
{
    auto p = std::make_shared<promise<R>>();
    future<R> future = p->get_future();
    while (d_ptr->_readyLock.test_and_set(std::memory_order_acquire));
    if(d_ptr->_isReady)
    {
        d_ptr->_readyLock.clear();
        call(p, func, d_ptr->_value);
    }
    else
    {
        d_ptr->_onReady = [p,func](T value){
            call(p, func, value);
        };
        d_ptr->_readyLock.clear();
    }
    return future;
}
template<typename Func, typename R>
future<R> future<void>::then(Func&& func)
{
    auto p = std::make_shared<promise<R>>();
    future<R> future = p->get_future();
    while (d_ptr->_readyLock.test_and_set(std::memory_order_acquire));
    if(d_ptr->_isReady)
    {
        d_ptr->_readyLock.clear();
        call(p, func);
    }
    else
    {
        d_ptr->_onReady = [p,func](){
            call(p, func);
        };
        d_ptr->_readyLock.clear();
    }
    return future;
}
}

#endif // FUTURE_H
