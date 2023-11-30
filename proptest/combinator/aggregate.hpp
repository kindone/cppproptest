#pragma once

#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../GenBase.hpp"
#include "../util/function_traits.hpp"
#include "../util/std.hpp"
#include "../generator/integral.hpp"

/**
 * @file aggregate.hpp
 * @brief Generator combinator for aggregating sequencially generated values into a value with a generator generator
 */

namespace proptest {

namespace util {

template <typename T>
Generator<T> aggregateImpl(GenFunction<T> gen1, function<GenFunction<T>(T&)> gen2gen, size_t minSize, size_t maxSize)
{
    auto gen1Ptr = util::make_shared<decltype(gen1)>(gen1);
    auto gen2genPtr = util::make_shared<function<GenFunction<T>(const T&)>>(
        [gen2gen](const T& t) { return gen2gen(const_cast<T&>(t)); });

    return interval<uint64_t>(minSize, maxSize).flatMap<T>([gen1Ptr, gen2genPtr](uint64_t& size) {
        return Generator<T>([gen1Ptr, gen2genPtr, size](Random& rand) {
            Shrinkable<T> shr = (*gen1Ptr)(rand);
            for (size_t i = 0; i < size; i++)
                shr = (*gen2genPtr)(shr.get())(rand);
            return shr;
        });
    });
}

}  // namespace util
/**
 * @ingroup Combinators
 * @brief Generator combinator for aggregating a value of type T from a generator generator
 * @tparam GEN1 Generator type of (Random&) -> Shrinkable<T>
 * @tparam GEN2GEN (T&) -> ((Random&) -> Shrinkable<T>) (Generator for T)
 * @param gen1 base generator for type T
 * @param gen2gen function that returns a generator for type T based on previously generated value of the same type
 * @param minSize minimum size of the aggregate steps
 * @param maxSize maximum size of the aggregate steps
 * @return last generated value of T throughout the aggregation
 */
template <typename GEN1, typename GEN2GEN>
    requires GenFunctionLike<GEN1, typename invoke_result_t<GEN1, Random&>::type> && GenFunctionLikeGen<GEN2GEN, typename invoke_result_t<GEN1, Random&>::type>
decltype(auto) aggregate(GEN1&& gen1, GEN2GEN&& gen2gen, size_t minSize, size_t maxSize)
{
    using T = typename invoke_result_t<GEN1, Random&>::type;  // get the T from shrinkable<T>(Random&)
    using RetType = invoke_result_t<GEN2GEN, T&>;             // GEN2GEN's return type
    GenFunction<T> funcGen1 = gen1;
    function<RetType(T&)> funcGen2Gen = gen2gen;
    return util::aggregateImpl<T>(funcGen1, funcGen2Gen, minSize, maxSize);
}

}  // namespace proptest
