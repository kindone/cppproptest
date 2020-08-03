#pragma once
#include <functional>
#include "../Shrinkable.hpp"

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);
template <typename T>
struct Generator;

template <typename T, typename GEN, typename Criteria>
decltype(auto) filter(GEN&& gen, Criteria&& criteria)
{
    auto genPtr = std::make_shared<GenFunction<T>>(std::forward<GEN>(gen));
    auto criteriaPtr =
        std::make_shared<std::function<bool(const T&)>>([criteria](const T& t) { return criteria(const_cast<T&>(t)); });
    return Generator<T>([criteriaPtr, genPtr](Random& rand) {
        while (true) {
            Shrinkable<T> shrinkable = (*genPtr)(rand);
            if ((*criteriaPtr)(shrinkable.getRef())) {
                return shrinkable.filter(criteriaPtr);
            }
        }
    });
}

// alias for filter
template <typename T, typename GEN, typename Criteria>
decltype(auto) suchThat(GEN&& gen, Criteria&& criteria)
{
    return filter<T, GEN, Criteria>(std::forward<GEN>(gen), std::forward<Criteria>(criteria));
}

}  // namespace proptest
