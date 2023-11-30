#pragma once
#include "../util/std.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "../GenBase.hpp"

/**
 * @file transform.hpp
 * @brief Generator combinator for generating a type U from a generator for type T by applying transformation on
 * generated value of type T
 */

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);
template <typename T>
struct Generator;

namespace util {

template <typename T, typename U>
struct TransformFunctor1 {
    TransformFunctor1(function<U(T&)> _transformer) : transformer(_transformer) { }

    Any operator()(const Any& a) {
        return Any(transformer(a.cast<T>()));
    }

    function<U(T&)> transformer;
};

template <typename T, typename U>
struct TransformFunctor2 {
    TransformFunctor2(shared_ptr<GenFunction<T>> _genPtr, shared_ptr<function<Any(const Any&)>> _transformerPtr)
        : genPtr(_genPtr), transformerPtr(_transformerPtr) {}

    Shrinkable<U> operator()(Random& rand) {
        Shrinkable<T> shrinkable = (*genPtr)(rand);
        return shrinkable.template map<U>(transformerPtr);
    }

    shared_ptr<GenFunction<T>> genPtr;
    shared_ptr<function<Any(const Any&)>> transformerPtr;
};

} // namespace util

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating a type U from a generator for type T by applying transformation on the
 * generated value of type T
 * @param gen generator for type T
 * @param transformer transformation function T& -> U
 */
template <typename T, typename U>
Generator<U> transform(GenFunction<T> gen, function<U(T&)> transformer)
{
    auto genPtr = util::make_shared<decltype(gen)>(gen);
    auto transformerPtr =
        util::make_shared<function<Any(const Any&)>>(util::TransformFunctor1<T, U>(transformer));
    return generator(util::TransformFunctor2<T, U>(genPtr, transformerPtr));
}

}  // namespace proptest
