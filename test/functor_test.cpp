#include "util/functor.h"
#include <experimental/any>
#include <cassert>
#include <vector>
#include <memory>

using std::experimental::any;
using std::experimental::any_cast;

namespace adapters
{
    template<typename T>
    struct adapter<T, any>
    {
        static T convert(any a)
        {
            return any_cast<T>(a);
        }
    };
    template<typename T>
    struct adapter<any, T>
    {
        static any convert(T t)
        {
            return any(t);
        }
    };
}

int main()
{
    auto functor = qflow::make_functor<any>([](std::string s, int i){
            assert(s == std::string("ahoj"));
            assert(i == 5);
            return 11;
    });
    
    std::vector<any> v = {std::string("ahoj"), 5};
    any a = functor.invoke(v);
    assert(any_cast<int>(a) == 11);


    auto functor2 = qflow::make_functor<any>([](std::string s, int i){
            assert(s == std::string("ahoj"));
            assert(i == 5);
    });
    any a2 = functor2.invoke(v);
    assert(a2.empty());


    auto functor3 = qflow::make_functor<any>([](){
            return 11;
    });
    any a3 = functor3.invoke();
    assert(any_cast<int>(a3) == 11);


    auto functor_ptr = qflow::make_functor_shared<any>([](std::string s, int i){
            assert(s == std::string("ahoj"));
            assert(i == 5);
            return 11;
    });
    any a4 = functor_ptr->invoke(v);
    assert(any_cast<int>(a4) == 11);
}
