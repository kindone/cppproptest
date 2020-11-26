#pragma once

#include <functional>
#include <tuple>
#include <stdexcept>
#include "util/function_traits.hpp"
#include "util/typelist.hpp"
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

class Random;

template <typename T>
struct Generator : public GenBase<T>
{
    Generator(GenFunction<T> gen) : genPtr(std::make_shared<GenFunction<T>>(gen)) {}

    virtual Shrinkable<T> operator()(Random& rand) override { return (*genPtr)(rand); }

    template <typename U>
    Generator<U> map(std::function<U(T&)> mapper)
    {
        auto thisPtr = clone();
        return Generator<U>(
            proptest::transform<T, U>([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); }, mapper));
    }

    template <typename Criteria>
    Generator<T> filter(Criteria&& criteria)
    {
        auto thisPtr = clone();
        return proptest::filter<T>([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); },
                                   std::forward<Criteria>(criteria));
    }

    template <typename U>
    Generator<std::pair<T, U>> pairWith(std::function<GenFunction<U>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::dependency<T, U>([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); }, gengen);
    }

    template <typename U>
    decltype(auto) tupleWith(std::function<GenFunction<U>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::chain([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); }, gengen);
    }

    template <typename U>
    Generator<U> flatMap(std::function<GenFunction<U>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::derive<T, U>([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); }, gengen);
    }

    std::shared_ptr<Generator<T>> clone() { return std::make_shared<Generator<T>>(*dynamic_cast<Generator<T>*>(this)); }

    std::shared_ptr<GenFunction<T>> genPtr;
};

template <typename GEN>
decltype(auto) generator(GEN&& gen)
{
    using RetType = typename function_traits<GEN>::return_type::type;  // cast Shrinkable<T>(Random&) -> T
    return Generator<RetType>(gen);
}

}  // namespace proptest
