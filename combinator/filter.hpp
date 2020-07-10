#pragma once
#include <functional>
#include "../Shrinkable.hpp"

namespace proptest {

template <typename GenT>
class Filter : public GenT {
public:
    using T = typename GenT::type;
    using FilterFunc = std::function<bool(T&)>;

    Filter(FilterFunc&& f) : filter(f) {}

    Shrinkable<T> operator()(Random& rand) override
    {
        while (true) {
            auto shrinkable = gen(rand);
            if (filter(shrinkable.getRef())) {
                return shrinkable;
            }
        }
    }

    GenT gen;
    FilterFunc filter;
};

template <typename T, typename GEN, typename Criteria>
decltype(auto) filter(GEN&& gen, Criteria&& criteria)
{
    auto genPtr = std::make_shared<std::function<Shrinkable<T>(Random&)>>(std::forward<GEN>(gen));
    auto criteriaPtr = std::make_shared<std::function<bool(const T&)>>(std::forward<Criteria>(criteria));
    return [criteriaPtr, genPtr](Random& rand) {
        while (true) {
            Shrinkable<T> shrinkable = (*genPtr)(rand);
            if ((*criteriaPtr)(shrinkable.getRef())) {
                return shrinkable.filter(criteriaPtr);
            }
        }
    };
}

// alias for filter
template <typename T, typename GEN, typename Criteria>
decltype(auto) suchThat(GEN&& gen, Criteria&& criteria)
{
    return filter<T, GEN, Criteria>(std::forward<GEN>(gen), std::forward<Criteria>(criteria));
}

}  // namespace proptest
