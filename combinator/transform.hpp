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
        util::make_shared<function<Any(const Any&)>>([transformer](const Any& a) {
            return Any(transformer(a.cast<T>()));
        });
    return generator([genPtr, transformerPtr](Random& rand) -> Shrinkable<U> {
        Shrinkable<T> shrinkable = (*genPtr)(rand);
        return shrinkable.template map<U>(transformerPtr);
    });
}

}  // namespace proptest
