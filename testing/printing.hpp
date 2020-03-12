#pragma once
#include <iostream>
#include <tuple>
#include <string>
#include <vector>
#include "testing/shrinkable.hpp"

namespace PropertyBasedTesting
{


std::ostream& show(std::ostream& os, const std::string&);
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

template <typename T>
std::ostream& show(std::ostream& os, const Shrinkable<T>& shrinkable);
template <typename T, typename Allocator>
std::ostream& show(std::ostream& os, const std::vector<T, Allocator>& vec);


struct HasShowImpl {
    template <typename T, typename CRITERIA = decltype(show(std::cout, std::declval<T>()))>
    static std::true_type test(const T&);

    static std::false_type test(...);

};

template <typename T>
struct HasShow {
    static constexpr bool value = decltype(HasShowImpl::test(std::declval<T>()))::value;
};

// default printer
template <typename T, bool = !HasShow<T>::value>
struct ShowDefault {
	static std::ostream& show(std::ostream& os, const T & obj)
	{
		os << "<\?\?\?>";
		return os;
	}
};

template <typename T>
struct ShowDefault<T, false> {
	static std::ostream& show(std::ostream& os, const T & obj)
	{
		os <<  obj;
		return os;
	}
};

template <typename T>
static std::ostream& show(std::ostream& os, const T & obj)
{
	ShowDefault<T>::show(os, obj);;
	return os;
}



template <typename T>
std::ostream& show(std::ostream& os, const Shrinkable<T>& shrinkable)
{
    show(os, static_cast<T>(shrinkable));
    return os;
}

template <typename T>
bool toStreamLast(std::ostream& os, const T& t)
{
    show(os, t);
    return true;
}

template <typename T>
bool toStreamFrontHelper(std::ostream& os, const T& t)
{
    show(os,  t);
    os << ", ";
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
std::ostream& show(std::ostream& os, const std::tuple<ARGS...>& tuple)
{
    constexpr auto Size = sizeof...(ARGS);
    os << "{ ";
    ToStreamEach<Size-1, const std::tuple<ARGS...>> toStreamEach;
    toStreamEach.get(os, std::move(tuple));
    toStreamLast(os, std::get<Size-1>(tuple));
    os << " }";
    return os;
}

template <typename T, typename Allocator>
std::ostream& show(std::ostream& os, const std::vector<T, Allocator>& vec) {
    os << "[ ";
	
	auto begin = vec.begin();
	if(begin == vec.end()) {
		show(os, *begin);
	}
	else {
		show(os, *begin);
		for(auto itr = ++begin; itr != vec.end(); itr++) {
			os << ", ";
			show(os, *itr);
		}
	}
    os << " ]";
    return os;
}



}
