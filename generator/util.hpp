#pragma once
#include "../api.hpp"
#include "../Shrinkable.hpp"
#include "../Stream.hpp"

namespace proptest {

class Random;

namespace util {

PROPTEST_API Shrinkable<int64_t> binarySearchShrinkable(int64_t value);
PROPTEST_API Shrinkable<uint64_t> binarySearchShrinkableU(uint64_t value);

template <typename T>
decltype(auto) GetShrinksHelper(const Shrinkable<T>& shr)
{
    return shr.shrinks();
}

template <typename T>
struct GetShrinks
{
    static Stream transform(T&& v) { return GetShrinksHelper(v); }
};

template <typename T>
struct Generate
{
    static decltype(auto) transform(T&& gen, Random& rand) { return gen(rand); }
};

template <typename T>
struct ShrinkableGet
{
    static decltype(auto) transform(T&& shrinkable) { return shrinkable.get(); }
};

template <typename T>
struct ShrinkableGetRef
{
    static decltype(auto) transform(T&& shrinkable) { return shrinkable.getRef(); }
};

}  // namespace util
}  // namespace proptest
