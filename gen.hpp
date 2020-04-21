#pragma once

#include <functional>
#include <tuple>
#include <stdexcept>
#include "function_traits.hpp"
#include "api.hpp"
#include "Random.hpp"
#include "typelist.hpp"
#include "Shrinkable.hpp"
#include "assert.hpp"
#include "Seq.hpp"

namespace PropertyBasedTesting
{

class Random;

template <typename T>
struct Gen
{
    using type = T;
    Gen() {
    }
    virtual ~Gen() {}

    virtual Shrinkable<T> operator()(Random& rand) {
        throw std::runtime_error("operator() should be defined for Gen");
    }
};

template <typename T, typename ElemGen = void>
struct Arbitrary : public Gen<T>{
};


} // namespace PropertyBasedTesting

#include "invokeWithArgs.hpp"
#include "invokeWithGenTuple.hpp"
#include "createGenTuple.hpp"

