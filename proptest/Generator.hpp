#pragma once

#include "util/std.hpp"
#include "util/function_traits.hpp"
#include "util/typelist.hpp"
#include "Random.hpp"
#include "Shrinkable.hpp"
#include "assert.hpp"
#include "combinator/transform.hpp"
#include "combinator/filter.hpp"
#include "combinator/dependency.hpp"
#include "combinator/chain.hpp"
#include "combinator/derive.hpp"
#include "GenBase.hpp"

/**
 * @file Generator.hpp
 * @brief Template definitions of Generator<T>
 */

namespace proptest {

class Random;

namespace util {

template <typename T>
struct GeneratorFunctor {
    GeneratorFunctor(shared_ptr<Generator<T>> ptr) : thisPtr(ptr) {}
    Shrinkable<T> operator()(Random& rand) { return (*thisPtr->genPtr)(rand); }
    shared_ptr<Generator<T>> thisPtr;
};

} // namespace util

/**
 * @ingroup Generators
 * @brief \ref Generator<T> is a wrapper for generator functions (or GenFunction<T>) to provide utility methods.
 * @details Generator<T> is the default result type of various utility methods of both Generator<T> and Arbi<T>
 */
template <typename T>
struct Generator : public GenBase<T>
{
    Generator(GenFunction<T> gen) : genPtr(util::make_shared<GenFunction<T>>(gen)) {}

    virtual Shrinkable<T> operator()(Random& rand) override { return (*genPtr)(rand); }

    /**
     * @brief Higher-order function that returns an altered Generator for type `U`, based on this Generator's generated
     * value of type `T`
     *
     * Similar to \ref flatMap, whereas the function in `map` returns a `U` but the function in `flatMap` returns a
     * `Generator<U>`. This gives greater simplicity
     *
     * @tparam U Target type
     * @param mapper Function that takes a value of type `T` and returns a value of type `U`
     * @return Generator<U> Generator for type `U`
     */
    template <typename U>
    Generator<U> map(function<U(T&)> mapper)
    {
        auto thisPtr = clone();
        return Generator<U>(
            proptest::transform<T, U>(util::GeneratorFunctor<T>(thisPtr), mapper));
    }

    /**
     * @brief Higher-order function that returns an altered Generator for type `U`,  based on this Generator's generated
     * value of type `T`
     *
     * @tparam F Callable type
     * @tparam U Target type
     * @param mapper Function that takes a value of type `T` and returns a value of type `U`
     * @return Generator<U> Generator for type `U`
     */
    template <invocable<T&> F>
    auto map(F&& mapper) -> Generator<invoke_result_t<F, T&>>
    {
        return map<invoke_result_t<F, T&>>(util::forward<F>(mapper));
    }

    /**
     * @brief Higher-order function that returns an altered Generator such that it restricts the values generated with
     * certain criteria
     *
     * @param criteria Function that takes a value `T` and returns whether to accept(true) or reject(false) the value
     * @return Generator<T> New Generator for type `T` that no longer includes the values that falsifies the criteria
     * function
     */
    Generator<T> filter(function<bool(T&)> criteria)
    {
        auto thisPtr = clone();
        return proptest::filter<T>(util::GeneratorFunctor<T>(thisPtr), criteria);
    }

    /**
     * @brief Higher-order function that lets you produce a pair of dependent generators, by taking a generated result
     * from this Generator
     *
     * @tparam U Target type, deduced automatically from genFactory
     * @param genFactory Function that takes a value of type `T` that would be generated by this Generator and returns
     * the next Generator for type `U`
     * @return Generator<pair<T, U>> New Generator would generate a pair of `T` and `U` types, where the second argument
     * of `U` type has dependency to the first argument of type `T`
     */
    template <typename U>
    Generator<pair<T, U>> pairWith(function<GenFunction<U>(T&)> genFactory)
    {
        auto thisPtr = clone();
        return proptest::dependency<T, U>(util::GeneratorFunctor<T>(thisPtr), genFactory);
    }

    template <invocable<T&> FACTORY>
    decltype(auto) pairWith(FACTORY&& genFactory)
    {
        using GEN = invoke_result_t<FACTORY, T&>;
        using RetType = typename invoke_result_t<GEN, Random&>::type;
        return pairWith<RetType>(util::forward<FACTORY>(genFactory));
    }

    /**
     * @brief Higher-order function that lets you produce a tuple of dependent generators, by taking a generated result
     * from this Generator
     *
     * @tparam U Next type, deduced automatically from genFactory
     * @param genFactory Function that takes a value of type `T` that would be generated by this Generator and returns
     * the next Generator for type `U`
     * @return Generator<tuple<T, U>> If T is not a tuple, new Generator would generate a tuple of `T` and `U` types,
     * where the second argument of `U` type has dependency to the first argument of type `T`
     * @return Generator<tuple<T1,...Tn, U>> If T is a tuple<T1,...,Tn>, new Generator would generate a tuple of
     * `T1,...,Tn` and `U` types, where the last argument of `U` type has dependency to the second last argument of `Tn`
     * type
     */
    template <typename U>
    decltype(auto) tupleWith(function<GenFunction<U>(T&)> genFactory)
    {
        auto thisPtr = clone();
        return proptest::chain(util::GeneratorFunctor<T>(thisPtr), genFactory);
    }

    template <typename FACTORY>
    decltype(auto) tupleWith(FACTORY&& genFactory)
    {
        using U = typename invoke_result_t<invoke_result_t<FACTORY, T&>, Random&>::type;
        return tupleWith<U>(util::forward<FACTORY>(genFactory));
    }

    /**
     * @brief Higher-order function that transforms the Generator for type `T` into a generator for type `U`
     *
     * Similar to `.map`, whereas the function in `map` returns a `U` but the function in `flatMap` returns a
     * `Generator<U>`. This gives higher freedom
     *
     * @tparam U Target Type
     * @param genFactory Function that takes a value of type `T` and returns a generator for type `U`
     * @return Generator<U> Generator for type `U`
     */
    template <typename U>
    Generator<U> flatMap(function<GenFunction<U>(T&)> genFactory)
    {
        auto thisPtr = clone();
        return proptest::derive<T, U>(util::GeneratorFunctor<T>(thisPtr), genFactory);
    }

    template <invocable<T&> FACTORY>
    decltype(auto) flatMap(FACTORY&& genFactory)
    {
        using U = typename invoke_result_t<invoke_result_t<FACTORY, T&>, Random&>::type;
        return flatMap<U>(util::forward(genFactory));
    }

    shared_ptr<Generator<T>> clone() { return util::make_shared<Generator<T>>(*dynamic_cast<Generator<T>*>(this)); }

    shared_ptr<GenFunction<T>> genPtr;
};

/**
 * @ingroup Generators
 * @brief Helper function to create \ref Generator<T> from generator functions to provide utility methods.
 */
template <typename GEN>
decltype(auto) generator(GEN&& gen)
{
    using RetType = typename function_traits<GEN>::return_type::type;  // cast Shrinkable<T>(Random&) -> T
    return Generator<RetType>(gen);
}

}  // namespace proptest