#pragma once
#include "testing/gen.hpp"

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

} // namespace
