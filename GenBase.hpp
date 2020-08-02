#pragma once

#include <functional>
#include <tuple>
#include <stdexcept>
#include "Random.hpp"
#include "Shrinkable.hpp"

namespace proptest {

class Random;

template <typename T>
using GenFunction = std::function<Shrinkable<T>(Random&)>;

template <typename T>
struct GenBase
{
    using type = T;

    virtual ~GenBase() {}

    virtual Shrinkable<T> operator()(Random&) { throw std::runtime_error("operator() should be defined for GenBase"); }
};

}  // namespace proptest
