#pragma once

#include <functional>
#include <tuple>
#include "testing/function_traits.hpp"
#include "testing/api.hpp"
#include "testing/Random.hpp"
#include "testing/typelist.hpp"
#include "testing/Shrinkable.hpp"
#include "testing/assert.hpp"
#include "testing/Seq.hpp"

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

    virtual Shrinkable<T> operator()(Random& rand) = 0;
};

template <typename T, typename ElemGen = void>
struct Arbitrary : public Gen<T>{
};


} // namespace PropertyBasedTesting

#include "testing/invokeWithArgs.hpp"
#include "testing/invokeWithGenTuple.hpp"
#include "testing/createGenTuple.hpp"

