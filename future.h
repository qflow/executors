#ifndef FUTURE_H
#define FUTURE_H

#include "executors_export.h"
#include <functional>
#include <future>
#include <atomic>

namespace qflow{

template<typename T>
class FutureBasePrivate
{
public:
    FutureBasePrivate() : _isReady(false)
    {

    }
    void set_value(T value)
    {
        while (_readyLock.test_and_set(std::memory_order_acquire));
        _value = value;
        _isReady = true;
        if(_onReady) _onReady(value);
        _readyLock.clear();
    }

    std::future<T> _stdFuture;
    std::function<void(T)> _onReady;
    bool _isReady;
    std::atomic_flag _readyLock = ATOMIC_FLAG_INIT;
    T _value;
};
template<>
class FutureBasePrivate<void>
{
public:
    FutureBasePrivate() : _isReady(false)
    {

    }
    void set_value()
    {
        while (_readyLock.test_and_set(std::memory_order_acquire));
        _isReady = true;
        if(_onReady) _onReady();
        _readyLock.clear();

    }
    std::future<void> _stdFuture;
    std::function<void()> _onReady;
    bool _isReady;
    std::atomic_flag _readyLock = ATOMIC_FLAG_INIT;
};

template<typename T>
class FutureBase;

template<>
class FutureBase<void>;

template<typename T>
class PromisePrivate
{
public:
    std::promise<T> _internal;
    std::shared_ptr<FutureBasePrivate<T>> _future;
};

template<typename T>
class Promise
{
public:
    Promise();
    Promise(const Promise<T>& other) = delete;
    Promise(Promise<T>&& other);
    FutureBase<T> get_future();
    void set_value( const T& value );
private:
    std::unique_ptr<PromisePrivate<T>> d_ptr;
};
template<>
class Promise<void>
{
public:
    Promise();
    Promise(const Promise<void>& other) = delete;
    Promise(Promise<void>&& other);
    FutureBase<void> get_future();
    void set_value();
private:
    std::unique_ptr<PromisePrivate<void>> d_ptr;
};

template<typename T>
class FutureBase
{
    friend class Promise<T>;
    using value_type = T;
public:
    FutureBase() : d_ptr(new FutureBasePrivate<T>())
    {

    }
    FutureBase(const FutureBase& other) = delete;
    FutureBase(FutureBase&& other) : FutureBase()
    {
        d_ptr = std::move(other.d_ptr);
    }

    FutureBase(std::future<T>&& other) : FutureBase()
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
    FutureBase<R> then(Func&& func);
protected:
    void set_value(T value)
    {
        d_ptr->set_value(value);
    }


    std::shared_ptr<FutureBasePrivate<T>> d_ptr;
};
template<>
class FutureBase<void>
{
    friend class Promise<void>;
public:
    FutureBase() : d_ptr(new FutureBasePrivate<void>())
    {

    }
    FutureBase(std::future<void>&& other) : FutureBase()
    {
        d_ptr->_stdFuture = std::move(other);
    }
    FutureBase(const FutureBase& other) = delete;
    FutureBase(FutureBase&& other) : FutureBase()
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
    FutureBase<R> then(Func&& func);
protected:
    void set_value()
    {
        d_ptr->set_value();

    }
    std::shared_ptr<FutureBasePrivate<void>> d_ptr;
};

template<typename T>
Promise<T>::Promise() : d_ptr(new PromisePrivate<T>())
{

}
template<typename T>
Promise<T>::Promise(Promise<T>&& other) : d_ptr(std::move(other.d_ptr))
{

}
template<typename T>
FutureBase<T> Promise<T>::get_future()
{
    std::future<T> stdFuture = d_ptr->_internal.get_future();
    FutureBase<T> ret(std::move(stdFuture));
    d_ptr->_future = ret.d_ptr;
    return ret;
}
template<typename T>
void Promise<T>::set_value( const T& value )
{
    d_ptr->_internal.set_value(value);
    d_ptr->_future->set_value(value);
}

template<typename R, typename T, typename Func>
void call(std::shared_ptr<Promise<R>> promise, Func&& func, T value)
{
    R resVal = func(value);
    promise->set_value(resVal);
}
template<typename R, typename Func>
void call(std::shared_ptr<Promise<R>> promise, Func&& func)
{
    R resVal = func();
    promise->set_value(resVal);
}
template<typename T, typename Func>
void call(std::shared_ptr<Promise<void>> promise, Func&& func, T value)
{
    func(value);
    promise->set_value();
}
template<typename Func>
void call(std::shared_ptr<Promise<void>> promise, Func&& func)
{
    func();
    promise->set_value();
}

template<typename T>
template<typename Func, typename R>
FutureBase<R> FutureBase<T>::then(Func&& func)
{
    auto promise = std::make_shared<Promise<R>>();
    FutureBase<R> future = promise->get_future();
    while (d_ptr->_readyLock.test_and_set(std::memory_order_acquire));
    if(d_ptr->_isReady)
    {
        call(promise, func, d_ptr->_value);
    }
    else
    {
        d_ptr->_onReady = [promise,func](T value){
            call(promise, func, value);
        };
    }
    d_ptr->_readyLock.clear();
    return future;
}
template<typename Func, typename R>
FutureBase<R> FutureBase<void>::then(Func&& func)
{
    auto promise = std::make_shared<Promise<R>>();
    FutureBase<R> future = promise->get_future();
    while (d_ptr->_readyLock.test_and_set(std::memory_order_acquire));
    if(d_ptr->_isReady)
    {
        call(promise, func);
    }
    else
    {
        d_ptr->_onReady = [promise,func](){
            call(promise, func);
        };
    }
    d_ptr->_readyLock.clear();
    return future;
}
}

#endif // FUTURE_H
