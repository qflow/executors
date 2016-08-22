#ifndef TEMPLATE_TYPE_H
#define TEMPLATE_TYPE_H

#include <type_traits>

template <typename C>
struct template_type;

template <template <typename > class C, typename T>
struct template_type<C<T>>
{
    using type = T;
};
template <typename T>
using get_template_type_t = typename template_type<std::remove_reference_t<T>>::type;

#endif
