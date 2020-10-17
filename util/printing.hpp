#pragma once
#include <iostream>
#include <tuple>
#include <utility>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include "../Shrinkable.hpp"
#include "utf8string.hpp"
#include "utf16string.hpp"
#include "cesu8string.hpp"
#include "nullable.hpp"

namespace proptest {

std::ostream& show(std::ostream& os, const char*);
std::ostream& show(std::ostream& os, const char*, size_t len);
std::ostream& show(std::ostream& os, const std::string&);
std::ostream& show(std::ostream& os, const UTF8String&);
std::ostream& show(std::ostream& os, const CESU8String&);
std::ostream& show(std::ostream& os, const UTF16BEString&);
std::ostream& show(std::ostream& os, const UTF16LEString&);
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
std::ostream& show(std::ostream& os, const T& obj);
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
template <class Key, class T, class Compare, class Allocator>
std::ostream& show(std::ostream& os, const std::map<Key, T, Compare, Allocator>& input);
template <typename T>
std::ostream& show(std::ostream& os, const std::shared_ptr<T>& ptr);
template <typename T>
std::ostream& show(std::ostream& os, const Nullable<T>& nullable);

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
template <typename T>
struct ShowDefault;
template <typename T>
struct ShowDefault
{
    static std::ostream& show(std::ostream& os, const T&)
    {
        os << "<\?\?\?>";
        return os;
    }
};

}  // namespace util

template <typename T>
std::ostream& show(std::ostream& os, const T& obj)
{
    util::ShowDefault<T>::show(os, obj);
    return os;
}

template <typename T>
std::ostream& show(std::ostream& os, const Shrinkable<T>& shrinkable)
{
    show(os, shrinkable.getRef());
    return os;
}

template <typename T>
struct Show
{
    Show(const T& _value) : value(_value) {}
    friend std::ostream& operator<<(std::ostream& os, const Show<T>& sh)
    {
        show(os, sh.value);
        return os;
    }
    const T& value;
};

template <>
struct Show<char*>
{
    Show(const char* _value, size_t _n) : value(_value), n(_n) {}
    friend std::ostream& operator<<(std::ostream& os, const Show<char*>& sh)
    {
        show(os, sh.value, sh.n);
        return os;
    }
    const char* value;
    size_t n;
};

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
    os << Show<T>(t) << ", ";
    return true;
}

template <typename Tuple, size_t... index>
decltype(auto) toStreamFront(std::ostream& os, const Tuple& tuple, std::index_sequence<index...>)
{
    [[maybe_unused]] auto dummy = {toStreamFrontHelper(os, std::get<index>(tuple))...};
}

template <size_t Size, typename Tuple>
struct ToStreamEach
{
    void get(std::ostream& os, const Tuple& tuple) { toStreamFront(os, tuple, std::make_index_sequence<Size>{}); }
};

template <typename Tuple>
struct ToStreamEach<0, Tuple>
{
    void get(std::ostream&, const Tuple&) {}
};

}  // namespace util

template <typename ARG1, typename ARG2>
std::ostream& show(std::ostream& os, const std::pair<ARG1, ARG2>& pair)
{
    os << "( " << Show<ARG1>(pair.first) << ", " << Show<ARG2>(pair.second) << " )";
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
        os << Show<T>(*begin);
        for (auto itr = ++begin; itr != seq.end(); itr++) {
            os << ", " << Show<T>(*itr);
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
        os << Show<T>(*begin);
        for (auto itr = ++begin; itr != seq.end(); itr++) {
            os << ", " << Show<T>(*itr);
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

template <typename Key, typename T, typename Compare, typename Allocator>
std::ostream& show(std::ostream& os, const std::map<Key, T, Compare, Allocator>& input)
{
    os << "{ ";
    if (input.size() == 1) {
        os << Show<std::pair<Key, T>>(*input.begin());
    } else if (input.size() > 0) {
        os << Show<std::pair<Key, T>>(*input.begin());
        auto second = input.begin();
        second++;
        for (auto itr = second; itr != input.end(); ++itr) {
            os << ", " << Show<std::pair<Key, T>>(*itr);
        }
    }

    os << " }";
    return os;
}

template <typename T>
std::ostream& show(std::ostream& os, const std::shared_ptr<T>& ptr)
{
    if (static_cast<bool>(ptr))
        os << Show<T>(*ptr);
    else
        os << "(null)";
    return os;
}

template <typename T>
std::ostream& show(std::ostream& os, const Nullable<T>& nullable)
{
    if (!nullable.isNull())
        os << Show<T>(*nullable.ptr);
    else
        os << "(null)";
    return os;
}

}  // namespace proptest
