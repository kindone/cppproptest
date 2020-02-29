#ifndef TESTING_TYPELIST_HPP
#define TESTING_TYPELIST_HPP


template <typename T>
struct TypeHolder {
    using type = T;
};

template<class... Ts> struct TypeList {
    using type_tuple = typename std::tuple<Ts...>;
};

template<class First, class... Ts> struct TypeList<First, Ts...> {
    using type_tuple = typename std::tuple<First, Ts...>;
    using Tail = TypeList<Ts...>;
};


namespace TypeListItemAt
{
    template<class List, int Index> struct AtHelper;
    template<template<class...> class List, class First, class... Rest>
    struct AtHelper<List<First, Rest...>, 0>
    {
        using type= First;
    };
    template<template<class...> class List, class First, class... Rest, int Index>
    struct AtHelper<List<First, Rest...>, Index>
    {
        using type= typename AtHelper<List<Rest...>, Index - 1>::type;
    };
    template<class, int> struct At;
    template<template<class...> class List, class... Ts, int Index>
    struct At<List<Ts...>, Index>
    {
        static_assert(Index >= 0, "Index connot be negative.");
        static_assert(Index < static_cast<int>(sizeof...(Ts)), "List "
                                                               "has "
                                                               "less "
                                                               "than N "
                                                               "elems"
                                                               ".");
        using type= typename AtHelper<List<Ts...>, Index>::type;
    };


}  // namespace At

template<class List, int Index>
using itemAt= typename TypeListItemAt::At<List, Index>::type;


#endif

