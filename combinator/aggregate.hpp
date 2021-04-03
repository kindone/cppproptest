#pragma once

#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../GenBase.hpp"
#include "../util/function_traits.hpp"
#include "../util/std.hpp"
#include "../generator/integral.hpp"

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

template <typename GEN1, typename GEN2GEN>
decltype(auto) aggregate(GEN1&& gen1, GEN2GEN&& gen2gen, size_t minSize, size_t maxSize)
{
    using CHAIN = typename function_traits<GEN1>::return_type::type;  // get the T from shrinkable<T>(Random&)
    using RetType = typename function_traits<GEN2GEN>::return_type;
    GenFunction<CHAIN> funcGen1 = gen1;
    function<RetType(CHAIN&)> funcGen2Gen = gen2gen;
    return util::aggregateImpl<CHAIN>(funcGen1, funcGen2Gen, minSize, maxSize);
}

}  // namespace proptest
