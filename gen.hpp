#pragma once

#include <functional>
#include <tuple>
#include <stdexcept>
#include "util/function_traits.hpp"
#include "util/typelist.hpp"
#include "api.hpp"
#include "Random.hpp"
#include "Shrinkable.hpp"
#include "assert.hpp"

namespace PropertyBasedTesting {

class Random;

template <typename T>
struct Gen
{
    using type = T;
    Gen() {}
    virtual ~Gen() {}

    virtual Shrinkable<T> operator()(Random& rand) { throw std::runtime_error("operator() should be defined for Gen"); }
};

template <typename T>
struct Arbitrary : public Gen<T>
{
};

}  // namespace PropertyBasedTesting

#include "util/invokeWithArgs.hpp"
#include "util/invokeWithGenTuple.hpp"
#include "util/createGenTuple.hpp"
