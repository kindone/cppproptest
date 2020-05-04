#ifndef TESTING_TYPELIST_HPP
#define TESTING_TYPELIST_HPP

#include <tuple>

template <typename T>
struct TypeHolder
{
    using type = T;
};

template <class... Ts>
struct TypeList
{
    using type_tuple = typename std::tuple<Ts...>;
};

template <class First, class... Ts>
struct TypeList<First, Ts...>
{
    using type_tuple = typename std::tuple<First, Ts...>;
    using Tail = TypeList<Ts...>;
};

namespace TypeListItemAt {
template <class List, size_t Index>
struct AtHelper;
template <template <class...> class List, class First, class... Rest>
struct AtHelper<List<First, Rest...>, 0>
{
    using type = First;
};
template <template <class...> class List, class First, class... Rest, size_t Index>
struct AtHelper<List<First, Rest...>, Index>
{
    using type = typename AtHelper<List<Rest...>, Index - 1>::type;
};
template <class, size_t>
struct At;
template <template <class...> class List, class... Ts, size_t Index>
struct At<List<Ts...>, Index>
{
    static_assert(Index < sizeof...(Ts),
                  "List "
                  "has "
                  "less "
                  "than N "
                  "elems"
                  ".");
    using type = typename AtHelper<List<Ts...>, Index>::type;
};

}  // namespace TypeListItemAt

template <class List, size_t Index>
using itemAt = typename TypeListItemAt::At<List, Index>::type;

#endif
