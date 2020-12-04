#pragma once

#include "util/std.hpp"
#include "Random.hpp"
#include "Shrinkable.hpp"

namespace proptest {

class Random;

template <typename T>
using GenFunction = function<Shrinkable<T>(Random&)>;

template <typename T>
struct GenBase
{
    using type = T;

    virtual ~GenBase() {}

    virtual Shrinkable<T> operator()(Random&) { throw runtime_error("operator() should be defined for GenBase"); }
};

}  // namespace proptest
