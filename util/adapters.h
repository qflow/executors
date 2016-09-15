#ifndef ADAPTERS_H
#define ADAPTERS_H

namespace adapters
{
    template<typename A, typename B, typename Enable = void>
    struct adapter
    {
    };
    template<typename A, typename B>
    A as(B b)
    {
        return adapter<A, B>::convert(std::forward<B>(b));
    }
}

#endif
