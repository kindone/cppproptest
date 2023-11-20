#pragma once
#include "proptest/util/std.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/Random.hpp"
#include "proptest/GenBase.hpp"

/**
 * @file derive.hpp
 * @brief Generator combinator for chaining two generators to generate a pair of values, where the second generator
 * depends on generated value from the first generator
 */

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);
template <typename T>
struct Generator;

// returns a shrinkable pair of <T,U> where U depends on T
/**
 * @ingroup Combinators
 * @brief Generator combinator for chaining two generators to generate a pair of values, where the second generator
 * depends on generated value from the first generator. Serves similar purpose as \ref chain() and the only difference
 * is in the chained type (pair vs. tuple).
 * @details Generates a tuple<T,U> with dependency.  Generator for U is decided by T value
 * @code
 *     GenFunction<pair<T,U>> pairGen = derive(intGen, [](int& intVal) {
 *         auto stringGen = Arbi<string>();
 *         stringGen.setMaxSize(intVal); // string size is dependent to intVal generated from intGen
 *         return intVal;
 *     });
 *     // derive(gen, ...) is equivalent to gen.pairWith(...), if gen is of Arbitrary or Generator type
 * @endcode
 */
template <typename T, typename U>
Generator<U> derive(GenFunction<T> gen1, function<GenFunction<U>(T&)> gen2gen)
{
    auto gen1Ptr = util::make_shared<decltype(gen1)>(gen1);
    auto gen2genPtr = util::make_shared<function<Generator<U>(const T&)>>(
        [gen2gen](const T& t) { return gen2gen(const_cast<T&>(t)); });

    auto genU = [gen1Ptr, gen2genPtr](Random& rand) -> Shrinkable<U> {
        // generate T
        Shrinkable<T> shrinkableT = (*gen1Ptr)(rand);
        using Intermediate = pair<T, Shrinkable<U>>;

        // shrink strategy 1: expand Shrinkable<T>
        Shrinkable<pair<T, Shrinkable<U>>> intermediate =
            shrinkableT.template flatMap<pair<T, Shrinkable<U>>>([&rand, gen2genPtr](const T& t) {
                // generate U
                auto gen2 = (*gen2genPtr)(t);
                Shrinkable<U> shrinkableU = gen2(rand);
                return make_shrinkable<pair<T, Shrinkable<U>>>(util::make_pair(t, shrinkableU));
            });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const Shrinkable<Intermediate>& interShr) -> Stream {
                // assume interShr has no shrinks
                Intermediate& interpair = interShr.getRef();
                T& t = interpair.first;
                Shrinkable<U>& shrinkableU = interpair.second;
                Shrinkable<Intermediate> newShrinkableU =
                    shrinkableU.template flatMap<Intermediate>([t](const U& u) mutable {
                        return make_shrinkable<pair<T, Shrinkable<U>>>(util::make_pair(t, make_shrinkable<U>(u)));
                    });
                return newShrinkableU.shrinks();
            });

        // reformat pair<T, Shrinkable<U>> to U
        return intermediate.template flatMap<U>(
            +[](const Intermediate& interpair) -> Shrinkable<U> { return interpair.second; });
    };

    return generator(genU);
}

}  // namespace proptest
