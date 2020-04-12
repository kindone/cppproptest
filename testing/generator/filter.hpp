#pragma once
#include <functional>
#include "testing/Shrinkable.hpp"


namespace PropertyBasedTesting {

template <typename GenT>
class Filter : public GenT {
public:
    using T = typename GenT::type;
    using FilterFunc = std::function<bool(T&)>;

    Filter(FilterFunc&& f) : filter(f) {
    }

    Shrinkable<T> operator()(Random& rand) {
        while(true) {
            auto shrinkable = gen(rand);
            if(filter(shrinkable.getRef())) {
                return shrinkable;
            }
        }
    }

    GenT gen;
    FilterFunc filter;
};


template <typename T, typename GEN>
decltype(auto) filter(GEN&& gen, std::function<bool(const T&)> criteria) {
    return [criteria, &gen](Random& rand) {
        while(true) {
            Shrinkable<T> shrinkable = gen(rand);
            if(criteria(shrinkable.getRef())) {
                return shrinkable.filter(criteria);
            }
        }
    };
}

}
