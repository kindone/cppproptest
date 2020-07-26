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

namespace proptest {

class Random;

template <typename T>
struct GenBase
{
    using type = T;

    virtual ~GenBase() {}

    virtual Shrinkable<T> operator()(Random&) { throw std::runtime_error("operator() should be defined for GenBase"); }
};

template <typename T>
struct Generator : public GenBase<T>
{
    Generator(std::function<Shrinkable<T>(Random&)> gen)
        : genPtr(std::make_shared<std::function<Shrinkable<T>(Random&)>>(gen))
    {
    }

    virtual Shrinkable<T> operator()(Random& rand) { return (*genPtr)(rand); }

    template <typename U>
    Generator<U> transform(std::function<U(T&)> transformer)
    {
        auto thisPtr = clone();
        return Generator<U>(
            proptest::transform<T, U>([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); }, transformer));
    }

    template <typename Criteria>
    Generator<T> filter(Criteria&& criteria)
    {
        auto thisPtr = clone();
        return proptest::filter<T>([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); },
                                   std::forward<Criteria>(criteria));
    }

    template <typename U>
    Generator<std::pair<T, U>> dependency(std::function<std::function<Shrinkable<U>(Random&)>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::dependency<T, U>([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); }, gengen);
    }

    template <typename U>
    Generator<std::tuple<T, U>> chain(std::function<std::function<Shrinkable<U>(Random&)>(T&)> gengen)
    {
        auto thisPtr = clone();
        auto pairGen = proptest::dependency<T, U>([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); }, gengen);
        return pairGen.template transform<std::tuple<T, U>>(
            [](const std::pair<T, U>& pair) { return std::make_tuple(pair.first, pair.second); });
    }

    std::shared_ptr<Generator<T>> clone() { return std::make_shared<Generator<T>>(*dynamic_cast<Generator<T>*>(this)); }

    std::shared_ptr<std::function<Shrinkable<T>(Random&)>> genPtr;
};

template <typename T>
struct Arbitrary;

template <typename T>
struct ArbitraryBase : public GenBase<T>
{
    template <typename U>
    Generator<U> transform(std::function<U(T&)> transformer)
    {
        auto thisPtr = clone();
        return proptest::transform<T, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, transformer);
    }

    template <typename Criteria>
    Generator<T> filter(Criteria&& criteria)
    {
        auto thisPtr = clone();
        return proptest::filter<T>([thisPtr](Random& rand) { return thisPtr->operator()(rand); },
                                   std::forward<Criteria>(criteria));
    }

    template <typename U>
    Generator<std::pair<T, U>> dependency(std::function<std::function<Shrinkable<U>(Random&)>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::dependency<T, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, gengen);
    }

    template <typename U>
    Generator<std::tuple<T, U>> chain(std::function<std::function<Shrinkable<U>(Random&)>(T&)> gengen)
    {
        auto thisPtr = clone();
        auto pairGen =
            proptest::dependency<T, U>([thisPtr](Random& rand) { return thisPtr->operator()(rand); }, gengen);
        return pairGen.template transform<std::tuple<T, U>>(
            [](const std::pair<T, U>& pair) { return std::make_tuple(pair.first, pair.second); });
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

template <typename GEN>
decltype(auto) generator(GEN&& gen)
{
    using RetType = typename function_traits<GEN>::return_type::type;  // cast Shrinkable<T>(Random&) -> T
    return Generator<RetType>(gen);
}

}  // namespace proptest

#include "util/invokeWithArgs.hpp"
#include "util/invokeWithGenTuple.hpp"
#include "util/createGenTuple.hpp"
