#ifndef ADAPTERS_H
#define ADAPTERS_H

#include <utility>

namespace adapters
{
    template<typename A, typename B, typename Enable = void>
    struct adapter
    {
        static A convert(B b);
    };
    template<typename A, typename B>
    A as(B b)
    {
        return adapter<A, B>::convert(std::forward<B>(b));
    }
}

#endif
