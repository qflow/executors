#ifndef RESULT_OF_FRIENDLY_H
#define RESULT_OF_FRIENDLY_H

#include <type_traits>

template< class F, class T>
struct result_of_friendly
{
    using type = std::result_of_t<F(T)>;
};

template< class F>
struct result_of_friendly<F, void>
{
    using type = std::result_of_t<F()>;
};
template< class F, class T>
using result_of_friendly_t = typename result_of_friendly<F, T>::type;

#endif
