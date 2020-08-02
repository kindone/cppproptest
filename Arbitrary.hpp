#pragma once

#include <functional>
#include <tuple>
#include <stdexcept>
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
struct Arbitrary;

template <typename T>
struct ArbitraryBase : public GenBase<T>
{
    template <typename U>
    Generator<U> map(std::function<U(T&)> mapper)
    {
        auto thisPtr = clone();
        return proptest::transform<T, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, mapper);
    }

    template <typename Criteria>
    Generator<T> filter(Criteria&& criteria)
    {
        auto thisPtr = clone();
        return proptest::filter<T>([thisPtr](Random& rand) { return thisPtr->operator()(rand); },
                                   std::forward<Criteria>(criteria));
    }

    template <typename U>
    Generator<std::pair<T, U>> pair(std::function<GenFunction<U>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::dependency<T, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, gengen);
    }

    template <typename U>
    decltype(auto) tuple(std::function<GenFunction<U>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::chain([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, gengen);
    }

    template <typename U>
    Generator<U> flatMap(std::function<GenFunction<U>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::derive<T, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, gengen);
    }

    std::shared_ptr<Arbitrary<T>> clone() { return std::make_shared<Arbitrary<T>>(*dynamic_cast<Arbitrary<T>*>(this)); }
};

template <typename T>
struct ArbitraryContainer : public ArbitraryBase<T>
{
    ArbitraryContainer(size_t minSize, size_t maxSize) : minSize(minSize), maxSize(maxSize) {}

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
struct Arbitrary : public ArbitraryBase<T>
{
};

}  // namespace proptest
