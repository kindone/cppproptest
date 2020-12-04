#pragma once

#include "util/std.hpp"
#include "util/function_traits.hpp"
#include "util/typelist.hpp"
#include "api.hpp"
#include "Random.hpp"
#include "Shrinkable.hpp"
#include "assert.hpp"
#include "combinator/transform.hpp"
#include "combinator/filter.hpp"
#include "combinator/dependency.hpp"
#include "combinator/chain.hpp"
#include "combinator/derive.hpp"
#include "GenBase.hpp"

namespace proptest {

template <typename T>
struct Arbi;

template <typename T>
struct ArbiBase : public GenBase<T>
{
    template <typename U>
    Generator<U> map(function<U(T&)> mapper)
    {
        auto thisPtr = clone();
        return proptest::transform<T, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, mapper);
    }

    template <typename F, typename U = typename result_of<F(T&)>::type>
    auto map(F&& mapper) -> Generator<U>
    {
        return map<U>(forward<F>(mapper));
    }

    Generator<T> filter(function<bool(T&)> criteria)
    {
        auto thisPtr = clone();
        return proptest::filter<T>([thisPtr](Random& rand) { return thisPtr->operator()(rand); },
                                   criteria);
    }

    template <typename U>
    Generator<pair<T, U>> pairWith(function<GenFunction<U>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::dependency<T, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, gengen);
    }

    template <typename U>
    decltype(auto) tupleWith(function<GenFunction<U>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::chain([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, gengen);
    }

    template <typename U>
    Generator<U> flatMap(function<GenFunction<U>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::derive<T, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, gengen);
    }

    shared_ptr<Arbi<T>> clone() { return make_shared<Arbi<T>>(*dynamic_cast<Arbi<T>*>(this)); }
};

template <typename T>
struct ArbiContainer : public ArbiBase<T>
{
    ArbiContainer(size_t _minSize, size_t _maxSize) : minSize(_minSize), maxSize(_maxSize) {}

    void setMinSize(size_t size) { minSize = size; }

    void setMaxSize(size_t size) { maxSize = size; }

    void setSize(size_t size)
    {
        minSize = size;
        maxSize = size;
    }

    void setSize(size_t min, size_t max)
    {
        minSize = min;
        maxSize = max;
    }

    size_t minSize;
    size_t maxSize;
};

template <typename T>
struct Arbi : public ArbiBase<T>
{
};

/* Aliases */
template <typename...ARGS>
using Arbitrary = Arbi<ARGS...>;

template <typename...ARGS>
using ArbitraryBase = ArbiBase<ARGS...>;

template <typename...ARGS>
using ArbitraryContainer = ArbiContainer<ARGS...>;

}  // namespace proptest
