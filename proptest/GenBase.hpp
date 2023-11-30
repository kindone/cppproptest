#pragma once

#include "util/std.hpp"
#include "Random.hpp"
#include "Shrinkable.hpp"

/** @file GenBase.hpp
 *  @brief Defines fundamental generator types
 */

namespace proptest {

class Random;
template <typename T> class Shrinkable;
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

template <typename F, typename T, typename S = T>
concept GenFunctionLike = requires(F f, Random& rand) {
    { f(rand) }
    -> same_as<Shrinkable<S>>;
};

template <typename F, typename T>
concept GenFunctionLikeGen = requires(F f, T& t) {
    { f(t) }
    -> GenFunctionLike<T, typename invoke_result_t<invoke_result_t<F, T&>, Random&>::type>;
};

template <typename T>
struct GenBase
{
    using type = T;

    virtual ~GenBase() = default;

    virtual Shrinkable<T> operator()(Random&) { throw runtime_error("operator() should be defined for GenBase"); }
};

}  // namespace proptest
