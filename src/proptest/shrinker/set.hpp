#pragma once
#include "proptest/Shrinkable.hpp"
#include "proptest/util/std.hpp"
#include "proptest/shrinker/listlike.hpp"

namespace proptest {

template <typename T>
Shrinkable<set<T>> shrinkSet(const shared_ptr<set<Shrinkable<T>>>& shrinkableSet, size_t minSize) {
    return shrinkContainer<set, T>(shrinkableSet, minSize);
}

}
