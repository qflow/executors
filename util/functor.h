#ifndef FUNCTOR_H
#define FUNCTOR_H


#include <functional>
#include <vector>
#include <util/function_traits.h>
#include <memory>

namespace qflow{

template<typename result_type, typename variant_type>
struct apply
{
    template <class Tuple, class Function, size_t... Is>
    variant_type exec(std::vector<variant_type> args,
                              std::index_sequence<Is...>, Function&& func) {
        result_type ret = func(adapters::template as<std::tuple_element_t<Is, Tuple>>(args[Is])...);
        return adapters::template as<variant_type>(ret);
    }
};
template<typename variant_type>
struct apply<void, variant_type>
{
    template <class Tuple, class Function, size_t... Is>
    variant_type exec(std::vector<variant_type> args,
                              std::index_sequence<Is...>, Function&& func) {
        func(adapters::template as<std::tuple_element_t<Is, Tuple>>(args[Is])...);
        return variant_type();
    }
};

template<typename variant_type>
class functor
{
public:
    virtual variant_type invoke(std::vector<variant_type> args = std::vector<variant_type>()) = 0;

};
template<typename variant_type, typename Function>
class functor_impl : public functor<variant_type>
{
public:
    functor_impl(Function&& f) : func(f)
    {

    }
    functor_impl(const functor_impl<variant_type, Function>& other) : func(other.func)
    {

    }
    functor_impl(functor_impl&& other) : func(std::move(other.func))
    {

    }

    variant_type invoke(std::vector<variant_type> args = std::vector<variant_type>())
    {
        using arg_types = typename function_traits<Function>::args;
        arg_types at;
        using result_type = typename function_traits<Function>::result_type;
        apply<result_type, variant_type> a;
        return a.exec<arg_types>(args, std::make_index_sequence<std::tuple_size<arg_types>::value>{}, func);

    }
    Function func;
};
template<typename variant_type, typename Function>
functor_impl<variant_type, Function> make_functor(Function&& f)
{
    return functor_impl<variant_type, Function>(std::forward<Function>(f));
}
template<typename variant_type, typename Function>
std::shared_ptr<functor<variant_type>> make_functor_shared(Function&& f)
{
    functor<variant_type>* ptr = new functor_impl<variant_type, Function>(std::forward<Function>(f));
    return std::shared_ptr<functor<variant_type>>(ptr);
}
}
#endif
