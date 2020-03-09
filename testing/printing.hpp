#pragma once
#include <iostream>
#include <tuple>
#include <string>
#include "testing/shrinkable.hpp"

namespace PropertyBasedTesting
{
namespace printing {
// default printer
template <typename T>
typename std::enable_if<!std::is_same<T, std::string>::value, std::ostream&>::type operator<<(std::ostream& out, const T & obj)
{
    std::operator<<(out, "???");
    return out;
}

} // namespace printing


template <typename T>
std::ostream& operator<<(std::ostream& out, const Shrinkable<T>& shrinkable)
{
    using namespace printing;
    //std::operator<<(out, "Shrinkable(");
    out << shrinkable.value;
    //std::operator<<(out, ")");
    return out;
}

template <typename T>
bool toStreamLast(std::ostream& os, const T& t)
{
	using namespace printing;
    os << t;
    return true;
}

template <typename T>
bool toStreamFrontHelper(std::ostream& os, const T& t)
{
	using namespace printing;
    os << t;
    std::operator<<(os, ", ");
    return true;
}

template <typename Tuple, std::size_t...index>
void toStreamFront(std::ostream& os, Tuple&& tuple, std::index_sequence<index...>) {
    auto dummy = {toStreamFrontHelper(os, std::get<index>(tuple))...};
}

template <size_t Size, typename Tuple>
struct ToStreamEach {
    void get(std::ostream& os, Tuple&& tuple) {
        toStreamFront(os, std::move(tuple), std::make_index_sequence<Size>{});
    }
};

template <typename Tuple>
struct ToStreamEach<0,Tuple> {
    void get(std::ostream& os, Tuple&& tuple) {
    }
};

template <typename ...ARGS>
std::ostream& operator<< (std::ostream& os, const std::tuple<ARGS...>& tuple)
{
    std::operator<<(os, "{");

    constexpr auto Size = sizeof...(ARGS);
    ToStreamEach<Size-1, const std::tuple<ARGS...>> toStreamEach;
    toStreamEach.get(os, std::move(tuple));
    toStreamLast(os, std::get<Size-1>(tuple));
    std::operator<<(os, "}");

    return os;
}



}
