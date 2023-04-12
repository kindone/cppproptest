#pragma once

#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../GenBase.hpp"
#include "../util/function_traits.hpp"
#include "../util/std.hpp"
#include "../generator/integral.hpp"
#include "../shrinker/listlike.hpp"

/**
 * @file accumulate.hpp
 * @brief Generator combinator for accumulating values into a vector where each value is generated from a given
 * generator generator
 */

namespace proptest {

namespace util {

template <typename T>
Generator<vector<T>> accumulateImpl(GenFunction<T> gen1, function<GenFunction<T>(T&)> gen2gen, size_t minSize,
                                    size_t maxSize)
{
    auto gen1Ptr = util::make_shared<decltype(gen1)>(gen1);
    auto gen2genPtr = util::make_shared<function<GenFunction<T>(const T&)>>(
        [gen2gen](const T& t) { return gen2gen(const_cast<T&>(t)); });

    return interval<uint64_t>(minSize, maxSize).flatMap<vector<T>>([gen1Ptr, gen2genPtr, minSize](uint64_t& size) {
        if (size == 0)
            return Generator<vector<T>>([](Random&) { return make_shrinkable<vector<T>>(); });
        return Generator<vector<T>>([gen1Ptr, gen2genPtr, size, minSize](Random& rand) {
            Shrinkable<T> shr = (*gen1Ptr)(rand);
            auto shrVec = make_shared<vector<Shrinkable<T>>>();
            shrVec->reserve(size);
            shrVec->push_back(shr);
            for (size_t i = 1; i < size; i++) {
                shr = (*gen2genPtr)(shr.get())(rand);
                shrVec->push_back(shr);
            }
            return shrinkListLikeLength<vector, T>(shrVec, minSize)
                .andThen([](const Shrinkable<vector<Shrinkable<T>>>& parent) {
                    vector<Shrinkable<T>>& shrVec_ = parent.getRef();
                    if (shrVec_.size() == 0)
                        return Stream::empty();
                    Shrinkable<T>& lastElemShr = shrVec_.back();
                    Stream elemShrinks = lastElemShr.shrinks();
                    if (elemShrinks.isEmpty())
                        return Stream::empty();
                    return elemShrinks.template transform<ShrinkableAny, ShrinkableAny>(
                        [copy = shrVec_](const ShrinkableAny& elem) mutable -> ShrinkableAny {
                            copy[copy.size() - 1] = Shrinkable<T>(elem);
                            return make_shrinkable_any<vector<Shrinkable<T>>>(copy);
                        });
                })
                .template map<vector<T>>([](const vector<Shrinkable<T>>& shrVec) {
                    vector<T> valVec;
                    valVec.reserve(shrVec.size());
                    util::transform(shrVec.begin(), shrVec.end(), util::back_inserter(valVec),
                                    [](const Shrinkable<T>& shr) { return shr.get(); });
                    return valVec;
                });
        });
    });
}

}  // namespace util

/**
 * @ingroup Combinators
 * @brief Generator combinator for accumulating values into a vector where each value is generated from a given
 * generator generator
 * @tparam GEN1 Generator type of (Random&) -> Shrinkable<T>
 * @tparam GEN2GEN (T&) -> ((Random&) -> Shrinkable<T>) (Generator for T)
 * @param gen1 base generator for type T
 * @param gen2gen function that returns a generator for type T based on previously generated value of the same type
 * @param minSize minimum size of the aggregate
 * @param maxSize maximum size of the aggregate
 * @return vector of generated values of type T
 */
template <typename GEN1, typename GEN2GEN>
decltype(auto) accumulate(GEN1&& gen1, GEN2GEN&& gen2gen, size_t minSize, size_t maxSize)
{
    using T = typename invoke_result_t<GEN1, Random&>::type;  // get the T from shrinkable<T>(Random&)
    using RetType = invoke_result_t<GEN2GEN, T&>;
    GenFunction<T> funcGen1 = gen1;
    function<RetType(T&)> funcGen2Gen = gen2gen;
    return util::accumulateImpl<T>(funcGen1, funcGen2Gen, minSize, maxSize);
}

}  // namespace proptest
