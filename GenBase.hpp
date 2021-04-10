#pragma once

#include "util/std.hpp"
#include "Random.hpp"
#include "Shrinkable.hpp"

/** @file
 *  @brief Defines fundamental generator types
 */

namespace proptest {

class Random;
/**
 * @brief Standard function type for generators
 *
 * `GenFunction` is the standardized type for callables in the form of `(Random&) -> Shrinkable<T>`
 *
 * Usage:
 * @code
 *     // assign lambda of (Random&) -> Shrinkable<int> to GenFunction<int>
 *     GenFunction<int> intGen = [](Random& rand) {
 *        Shrinkable<int> shrinkableInt = make_shrinkable<int>(rand.getRandomInt32());
 *        return shrinkableInt;
 *     };
 *
 *     forAll([](int intVal) {
 *        // do stuff with intVal
 *     }, intGen);
 * @endcode
 * @tparam T The generated type
 */
template <typename T>
using GenFunction = function<Shrinkable<T>(Random&)>;

/// @private
template <typename T>
struct GenBase
{
    using type = T;

    virtual ~GenBase() = default;

    virtual Shrinkable<T> operator()(Random&) { throw runtime_error("operator() should be defined for GenBase"); }
};

}  // namespace proptest
