#pragma once

#include "util/std.hpp"
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
    Generator(GenFunction<T> gen) : genPtr(make_shared<GenFunction<T>>(gen)) {}

    virtual Shrinkable<T> operator()(Random& rand) override { return (*genPtr)(rand); }

    template <typename U>
    Generator<U> map(function<U(T&)> mapper)
    {
        auto thisPtr = clone();
        return Generator<U>(
            proptest::transform<T, U>([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); }, mapper));
    }

    template <typename F, typename U = typename result_of<F(T&)>::type>
    auto map(F&& mapper) -> Generator<U>
    {
        return map<U>(forward<F>(mapper));
    }

    Generator<T> filter(function<bool(T&)> criteria)
    {
        auto thisPtr = clone();
        return proptest::filter<T>([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); },
                                   criteria);
    }

    template <typename U>
    Generator<pair<T, U>> pairWith(function<GenFunction<U>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::dependency<T, U>([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); }, gengen);
    }

    template <typename U>
    decltype(auto) tupleWith(function<GenFunction<U>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::chain([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); }, gengen);
    }

    template <typename U>
    Generator<U> flatMap(function<GenFunction<U>(T&)> gengen)
    {
        auto thisPtr = clone();
        return proptest::derive<T, U>([thisPtr](Random& rand) { return (*thisPtr->genPtr)(rand); }, gengen);
    }

    shared_ptr<Generator<T>> clone() { return make_shared<Generator<T>>(*dynamic_cast<Generator<T>*>(this)); }

    shared_ptr<GenFunction<T>> genPtr;
};

template <typename GEN>
decltype(auto) generator(GEN&& gen)
{
    using RetType = typename function_traits<GEN>::return_type::type;  // cast Shrinkable<T>(Random&) -> T
    return Generator<RetType>(gen);
}

}  // namespace proptest
