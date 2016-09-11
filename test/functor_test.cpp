#include "util/functor.h"
#include "util/any.h"
#include <cassert>
#include <vector>
#include <memory>


namespace adapters
{
    template<typename T>
    struct adapter<T, linb::any>
    {
        static T convert(linb::any a)
        {
            return linb::any_cast<T>(a);
        }
    };
    template<typename T>
    struct adapter<linb::any, T>
    {
        static linb::any convert(T t)
        {
            return linb::any(t);
        }
    };
}

int main()
{
    auto functor = qflow::make_functor<linb::any>([](std::string s, int i){
            assert(s == std::string("ahoj"));
            assert(i == 5);
            return 11;
    });
    
    std::vector<linb::any> v = {std::string("ahoj"), 5};
    linb::any a = functor.invoke(v);
    assert(linb::any_cast<int>(a) == 11);


    auto functor2 = qflow::make_functor<linb::any>([](std::string s, int i){
            assert(s == std::string("ahoj"));
            assert(i == 5);
    });
    linb::any a2 = functor2.invoke(v);
    assert(a2.empty());


    auto functor3 = qflow::make_functor<linb::any>([](){
            return 11;
    });
    linb::any a3 = functor3.invoke();
    assert(linb::any_cast<int>(a3) == 11);


    auto functor_ptr = qflow::make_functor_shared<linb::any>([](std::string s, int i){
            assert(s == std::string("ahoj"));
            assert(i == 5);
            return 11;
    });
    linb::any a4 = functor_ptr->invoke(v);
    assert(linb::any_cast<int>(a4) == 11);
}
