#pragma once

#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../GenBase.hpp"
#include "../util/function_traits.hpp"
#include "../util/std.hpp"

/**
 * @file chain.hpp
 * @brief Generator combinator for chaining two generators to generate a tuple of values, where the second generator
 * depends on generated value from the first generator
 */

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);
template <typename T>
struct Generator;

template <class... Ts>
using Chain = tuple<Ts...>;

namespace util {

template <typename U, typename T>
Generator<Chain<T, U>> chainImpl(GenFunction<T> gen1, function<GenFunction<U>(T&)> gen2gen)
{
    auto gen1Ptr = util::make_shared<decltype(gen1)>(gen1);
    auto gen2genPtr = util::make_shared<function<GenFunction<U>(const T&)>>(
        [gen2gen](const T& t) { return gen2gen(const_cast<T&>(t)); });

    auto genTuple = [gen1Ptr, gen2genPtr](Random& rand) -> Shrinkable<Chain<T, U>> {
        // generate T
        Shrinkable<T> shrinkableTs = (*gen1Ptr)(rand);
        using Intermediate = pair<T, Shrinkable<U>>;

        // shrink strategy 1: expand Shrinkable<T>
        Shrinkable<pair<T, Shrinkable<U>>> intermediate =
            shrinkableTs.template flatMap<pair<T, Shrinkable<U>>>([&rand, gen2genPtr](const T& t) {
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

        // reformat pair<T, Shrinkable<U>> to Chain<T, U>
        return intermediate.template flatMap<Chain<T, U>>(
            +[](const Intermediate& interpair) -> Shrinkable<tuple<T, U>> {
                const T& t = interpair.first;
                return make_shrinkable<Chain<T, U>>(
                    tuple_cat(tuple<T>(t), util::make_tuple(interpair.second.getRef())));
            });
    };

    return generator(genTuple);
}

template <typename U, typename T0, typename T1, typename... Ts>
Generator<Chain<T0, T1, Ts..., U>> chainImpl(GenFunction<Chain<T0, T1, Ts...>> gen1,
                                             function<GenFunction<U>(Chain<T0, T1, Ts...>&)> gen2gen)
{
    auto gen1Ptr = util::make_shared<decltype(gen1)>(gen1);
    auto gen2genPtr = util::make_shared<function<GenFunction<U>(const Chain<T0, T1, Ts...>&)>>(
        [gen2gen](const Chain<T0, T1, Ts...>& ts) { return gen2gen(const_cast<Chain<T0, T1, Ts...>&>(ts)); });

    auto genTuple = [gen1Ptr, gen2genPtr](Random& rand) -> Shrinkable<Chain<T0, T1, Ts..., U>> {
        // generate T
        Shrinkable<Chain<T0, T1, Ts...>> shrinkableTs = (*gen1Ptr)(rand);
        using Intermediate = pair<Chain<T0, T1, Ts...>, Shrinkable<U>>;

        // shrink strategy 1: expand Shrinkable<tuple<Ts...>>
        Shrinkable<pair<Chain<T0, T1, Ts...>, Shrinkable<U>>> intermediate =
            shrinkableTs.template flatMap<pair<Chain<T0, T1, Ts...>, Shrinkable<U>>>(
                [&rand, gen2genPtr](const Chain<T0, T1, Ts...>& ts) {
                    // generate U
                    auto gen2 = (*gen2genPtr)(ts);
                    Shrinkable<U> shrinkableU = gen2(rand);
                    return make_shrinkable<pair<Chain<T0, T1, Ts...>, Shrinkable<U>>>(util::make_pair(ts, shrinkableU));
                });

        // shrink strategy 2: expand Shrinkable<U>
        intermediate =
            intermediate.andThen(+[](const Shrinkable<Intermediate>& interShr) -> Stream {
                // assume interShr has no shrinks
                Intermediate& interpair = interShr.getRef();
                Chain<T0, T1, Ts...>& ts = interpair.first;
                Shrinkable<U>& shrinkableU = interpair.second;
                Shrinkable<Intermediate> newShrinkableU =
                    shrinkableU.template flatMap<Intermediate>([ts](const U& u) mutable {
                        return make_shrinkable<pair<Chain<T0, T1, Ts...>, Shrinkable<U>>>(
                            util::make_pair(ts, make_shrinkable<U>(u)));
                    });
                return newShrinkableU.shrinks();
            });

        // reformat pair<Chain<T0, T1, Ts...>, Shrinkable<U>> to Chain<T0, T1, Ts..., U>
        return intermediate.template flatMap<Chain<T0, T1, Ts..., U>>(
            +[](const Intermediate& interpair) -> Shrinkable<tuple<T0, T1, Ts..., U>> {
                const Chain<T0, T1, Ts...>& ts = interpair.first;
                return make_shrinkable<Chain<T0, T1, Ts..., U>>(tuple_cat(ts, tuple<U>(interpair.second.getRef())));
            });
    };

    return generator(genTuple);
}

}  // namespace util

/**
 * @ingroup Combinators
 * @brief Generator combinator for chaining two generators to generate a tuple of values, where the second generator
 * depends on generated value from the first generator. Serves similar purpose as \ref derive, the only difference is in
 * the chained type (tuple).
 * @details Generates a tuple<T,U> with dependency.  Generator for U is decided by T value
 * @code
 *     GenFunction<tuple<T,U>> tupleGen = chain(intGen, [](int& intVal) {
 *         auto stringGen = Arbi<string>();
 *         stringGen.setMaxSize(intVal); // string size is dependent to intVal generated from intGen
 *         return intVal;
 *     });
 *     // chain(gen, ...) is equivalent to gen.tupleWith(...), if gen is of Arbitrary or Generator type
 * @endcode
 */
template <typename GEN1, typename GEN2GEN>
decltype(auto) chain(GEN1&& gen1, GEN2GEN&& gen2gen)
{
    using CHAIN = typename function_traits<GEN1>::return_type::type;  // get the T from shrinkable<T>(Random&)
    using RetType = typename function_traits<GEN2GEN>::return_type;
    GenFunction<CHAIN> funcGen1 = gen1;
    function<RetType(CHAIN&)> funcGen2Gen = gen2gen;
    return util::chainImpl(funcGen1, funcGen2Gen);
}

}  // namespace proptest
