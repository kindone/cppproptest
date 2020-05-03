#pragma once
#include <iostream>
#include <tuple>
#include <utility>
#include <string>
#include <list>
#include <vector>
#include <set>
#include "Shrinkable.hpp"

namespace PropertyBasedTesting {

std::ostream& show(std::ostream& os, const std::string&);
std::ostream& show(std::ostream& os, const bool&);
std::ostream& show(std::ostream& os, const char&);
std::ostream& show(std::ostream& os, const int8_t&);
std::ostream& show(std::ostream& os, const uint8_t&);
std::ostream& show(std::ostream& os, const int16_t&);
std::ostream& show(std::ostream& os, const uint16_t&);
std::ostream& show(std::ostream& os, const int32_t&);
std::ostream& show(std::ostream& os, const uint32_t&);
std::ostream& show(std::ostream& os, const int64_t&);
std::ostream& show(std::ostream& os, const uint64_t&);
std::ostream& show(std::ostream& os, const float&);
std::ostream& show(std::ostream& os, const double&);

// forward declaration is needed to be available at call sites
template <typename T>
std::ostream& show(std::ostream& os, const Shrinkable<T>& shrinkable);
template <typename... ARGS>
std::ostream& show(std::ostream& os, const std::tuple<ARGS...>& tuple);
template <typename ARG1, typename ARG2>
std::ostream& show(std::ostream& os, const std::pair<ARG1, ARG2>& pair);
template <typename T, typename Allocator>
std::ostream& show(std::ostream& os, const std::vector<T, Allocator>& vec);
template <typename T, typename Allocator>
std::ostream& show(std::ostream& os, const std::list<T, Allocator>& list);
template <typename T, typename Compare, typename Allocator>
std::ostream& show(std::ostream& os, const std::set<T, Compare, Allocator>& input);

namespace util {

struct HasShowImpl
{
    template <typename T, typename CRITERIA = decltype(show(std::cout, std::declval<T>()))>
    static std::true_type test(const T&);

    static std::false_type test(...);
};

template <typename T>
struct HasShow
{
    static constexpr bool value = decltype(HasShowImpl::test(std::declval<T>()))::value;
};

// default printer
template <typename T, bool = !HasShow<T>::value>
struct ShowDefault
{
    static std::ostream& show(std::ostream& os, const T& obj)
    {
        os << "<\?\?\?>";
        return os;
    }
};

template <typename T>
struct ShowDefault<T, false>
{
    static std::ostream& show(std::ostream& os, const T& obj)
    {
        os << obj;
        return os;
    }
};

}  // namespace util

template <typename T>
static std::ostream& show(std::ostream& os, const T& obj)
{
    util::ShowDefault<T>::show(os, obj);
    ;
    return os;
}

template <typename T>
std::ostream& show(std::ostream& os, const Shrinkable<T>& shrinkable)
{
    show(os, shrinkable.getRef());
    return os;
}

namespace util {

template <typename T>
bool toStreamLast(std::ostream& os, const T& t)
{
    show(os, t);
    return true;
}

template <typename T>
bool toStreamFrontHelper(std::ostream& os, const T& t)
{
    show(os, t);
    os << ", ";
    return true;
}

template <typename Tuple, std::size_t... index>
decltype(auto) toStreamFront(std::ostream& os, const Tuple& tuple, std::index_sequence<index...>)
{
    auto dummy = {toStreamFrontHelper(os, std::get<index>(tuple))...};
    return dummy;
}

template <size_t Size, typename Tuple>
struct ToStreamEach
{
    void get(std::ostream& os, const Tuple& tuple) { toStreamFront(os, tuple, std::make_index_sequence<Size>{}); }
};

template <typename Tuple>
struct ToStreamEach<0, Tuple>
{
    void get(std::ostream& os, const Tuple& tuple) {}
};

}  // namespace util

template <typename ARG1, typename ARG2>
std::ostream& show(std::ostream& os, const std::pair<ARG1, ARG2>& pair)
{
    os << "( ";
    show(os, pair.first);
    os << ", ";
    show(os, pair.second);
    os << " )";
    return os;
}

template <typename... ARGS>
std::ostream& show(std::ostream& os, const std::tuple<ARGS...>& tuple)
{
    constexpr auto Size = sizeof...(ARGS);
    os << "{ ";
    util::ToStreamEach<Size - 1, std::tuple<ARGS...>> toStreamEach;
    toStreamEach.get(os, tuple);
    util::toStreamLast(os, std::get<Size - 1>(tuple));
    os << " }";
    return os;
}

template <typename T, typename Allocator>
std::ostream& show(std::ostream& os, const std::vector<T, Allocator>& seq)
{
    os << "[ ";
    auto begin = seq.begin();
    if (begin != seq.end()) {
        show(os, *begin);
        for (auto itr = ++begin; itr != seq.end(); itr++) {
            os << ", ";
            show(os, *itr);
        }
    }
    os << " ]";
    return os;
}

template <typename T, typename Allocator>
std::ostream& show(std::ostream& os, const std::list<T, Allocator>& seq)
{
    os << "[ ";
    auto begin = seq.begin();
    if (begin != seq.end()) {
        show(os, *begin);
        for (auto itr = ++begin; itr != seq.end(); itr++) {
            os << ", ";
            show(os, *itr);
        }
    }
    os << " ]";
    return os;
}

template <typename T, typename Compare, typename Allocator>
std::ostream& show(std::ostream& os, const std::set<T, Compare, Allocator>& input)
{
    os << "{ ";
    if (input.size() == 1) {
        os << *input.begin();
    } else if (input.size() > 0) {
        os << *input.begin();
        auto second = input.begin();
        second++;
        for (auto itr = second; itr != input.end(); ++itr) {
            os << ", " << *itr;
        }
    }

    os << " }";
    return os;
}

}  // namespace PropertyBasedTesting
