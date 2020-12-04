#pragma once
#include "../util/std.hpp"
#include "../Shrinkable.hpp"
#include "../GenBase.hpp"

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);
template <typename T>
struct Generator;

template <typename T, typename GEN, typename Criteria>
decltype(auto) filter(GEN&& gen, Criteria&& criteria)
{
    auto genPtr = make_shared<GenFunction<T>>(forward<GEN>(gen));
    auto criteriaPtr =
        make_shared<function<bool(const T&)>>([criteria](const T& t) { return criteria(const_cast<T&>(t)); });
    return Generator<T>([criteriaPtr, genPtr](Random& rand) {
        // TODO: add some configurable termination criteria (e.g. maximum no. of attempts)
        while (true) {
            Shrinkable<T> shrinkable = (*genPtr)(rand);
            if ((*criteriaPtr)(shrinkable.getRef())) {
                return shrinkable.filter(criteriaPtr, 1); // 1: tolerance
            }
        }
    });
}

// alias for filter
template <typename T, typename GEN, typename Criteria>
decltype(auto) suchThat(GEN&& gen, Criteria&& criteria)
{
    return filter<T, GEN, Criteria>(forward<GEN>(gen), forward<Criteria>(criteria));
}

}  // namespace proptest
