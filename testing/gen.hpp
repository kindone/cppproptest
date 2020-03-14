#ifndef TESTING_GEN_HPP
#define TESTING_GEN_HPP
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

    Shrinkable<T> generate(Random& rand);
};

template <typename T, typename ElemGen = void>
struct Arbitrary : public Gen<T>{
};


template <class T> class BaseType;
template <class T> class BaseType<Gen<T>> {
public:
    using type = T;
};

template <class T> class BaseType<Arbitrary<T>> {
public:
    using type = T;
};



} // namespace PropertyBasedTesting

#include "testing/Map.hpp"
#include "testing/invokeWithArgs.hpp"
#include "testing/invokeWithGenTuple.hpp"
#include "testing/createGenTuple.hpp"

#endif // TESTING_GEN_HPP
