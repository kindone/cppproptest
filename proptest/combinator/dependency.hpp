#pragma once
#include "../util/std.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../GenBase.hpp"

/**
 * @file dependency.hpp
 * @brief Generator combinator for generating values with dependency or relation to a base generator
 */

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);
template <typename T>
struct Generator;

namespace util {

template <typename T, typename U>
struct DependencyCastFunctor
{
    DependencyCastFunctor(const function<GenFunction<U>(T&)>& _gen2gen) : gen2gen(_gen2gen) {}

    GenFunction<U> operator()(const T& t) { return gen2gen(const_cast<T&>(t)); }

    function<GenFunction<U>(T&)> gen2gen;
};

template <typename T, typename U>
struct DependencyInterFunctor {
    DependencyInterFunctor(Random& _rand, shared_ptr<function<GenFunction<U>(const T&)>> _gen2genPtr)
        : rand(_rand), gen2genPtr(_gen2genPtr) {}

    Shrinkable<pair<T, Shrinkable<U>>> operator()(const T& t) {
        // generate U
        auto gen2 = (*gen2genPtr)(t);
        Shrinkable<U> shrinkableU = gen2(rand);
        return make_shrinkable<pair<T, Shrinkable<U>>>(util::make_pair(t, shrinkableU));
    }

    Random& rand;
    shared_ptr<function<GenFunction<U>(const T&)>> gen2genPtr;
};

template <typename T, typename U>
struct DependencyFunctor
{
    DependencyFunctor(shared_ptr<GenFunction<T>> _gen1Ptr, shared_ptr<function<GenFunction<U>(const T&)>> _gen2genPtr)
        : gen1Ptr(_gen1Ptr), gen2genPtr(_gen2genPtr) {}

    Shrinkable<pair<T, U>> operator()(Random& rand) {
        // generate T
        Shrinkable<T> shrinkableT = (*gen1Ptr)(rand);
        using Intermediate = pair<T, Shrinkable<U>>;

        // shrink strategy 1: expand Shrinkable<T>
        Shrinkable<pair<T, Shrinkable<U>>> intermediate =
            shrinkableT.template flatMap<pair<T, Shrinkable<U>>>(util::DependencyInterFunctor<T, U>(rand, gen2genPtr));

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const Shrinkable<Intermediate>& interShr) -> Stream {
                // assume interShr has no shrinks
                shared_ptr<Intermediate> interpair = interShr.getSharedPtr();
                Shrinkable<U>& shrinkableU = interpair->second;
                Shrinkable<Intermediate> newShrinkableU =
                    shrinkableU.template flatMap<Intermediate>([interpair](const U& u) mutable {
                        return make_shrinkable<pair<T, Shrinkable<U>>>(
                            util::make_pair(interpair->first, make_shrinkable<U>(u)));
                    });
                return newShrinkableU.shrinks();
            });

        // reformat pair<T, Shrinkable<U>> to pair<T, U>
        return intermediate.template flatMap<pair<T, U>>(+[](const Intermediate& interpair) -> Shrinkable<pair<T, U>> {
            return make_shrinkable<pair<T, U>>(util::make_pair(interpair.first, interpair.second.getRef()));
        });
    }

    shared_ptr<GenFunction<T>> gen1Ptr;
    shared_ptr<function<GenFunction<U>(const T&)>> gen2genPtr;
};


} // namespace util

/**
 * @ingroup Combinators
 * @brief Generator combinator for generating values with dependencies or relation to a base generator
 * @param gen1 base generator
 * @param gen2gen generator generator that generates a value based on generated value from the base generator
 * @return template <typename T, typename U>
 */
// returns a shrinkable pair of <T,U> where U depends on T
template <typename T, typename U>
Generator<pair<T, U>> dependency(GenFunction<T> gen1, function<GenFunction<U>(T&)> gen2gen)
{
    auto gen1Ptr = util::make_shared<decltype(gen1)>(gen1);
    auto gen2genPtr = util::make_shared<function<GenFunction<U>(const T&)>>(util::DependencyCastFunctor<T, U>(gen2gen));
    return generator(util::DependencyFunctor<T, U>(gen1Ptr, gen2genPtr));
}

}  // namespace proptest
