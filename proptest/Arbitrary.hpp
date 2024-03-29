#pragma once

#include "util/std.hpp"
#include "util/function_traits.hpp"
#include "util/typelist.hpp"
#include "api.hpp"
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
 * @file Arbitrary.hpp
 * @brief Template definitions of Arbitrary<T> and its variants
 */
namespace proptest {

template <typename T>
struct Arbi;
template <typename T>
struct ArbiBase;

namespace util {

template <typename T>
struct ArbiFunctor {
    ArbiFunctor(shared_ptr<ArbiBase<T>> ptr) : thisPtr(ptr) {}
    Shrinkable<T> operator()(Random& rand) { return thisPtr->operator()(rand); }
    shared_ptr<ArbiBase<T>> thisPtr;
};

} // namespace util


/**
 * @ingroup Generators
 * @brief \ref ArbiBase provides basic functionality for \ref Arbi and \ref ArbiContainer. It provides utility methods.
 */
template <typename T>
struct ArbiBase : public GenBase<T>
{
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
        return proptest::transform<T, U>(util::ArbiFunctor<T>(thisPtr), mapper);
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
        return proptest::filter<T>(util::ArbiFunctor<T>(thisPtr), criteria);
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
        return proptest::dependency<T, U>(util::ArbiFunctor<T>(thisPtr), genFactory);
    }

    template <invocable<T&> FACTORY>
    decltype(auto) pairWith(FACTORY&& genFactory)
    {
        using U = typename invoke_result_t<invoke_result_t<FACTORY, T&>, Random&>::type;
        return pairWith<U>(util::forward<FACTORY>(genFactory));
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
        return proptest::chain(util::ArbiFunctor<T>(thisPtr), genFactory);
    }

    template <invocable<T&> FACTORY>
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
        return proptest::derive<T, U>(util::ArbiFunctor<T>(thisPtr), genFactory);
    }

    template <invocable<T&> FACTORY>
    decltype(auto) flatMap(FACTORY&& genFactory)
    {
        using U = typename invoke_result_t<invoke_result_t<FACTORY, T&>, Random&>::type;
        return flatMap<U>(util::forward<FACTORY>(genFactory));
    }

    /// * private
    shared_ptr<Arbi<T>> clone() { return util::make_shared<Arbi<T>>(*dynamic_cast<Arbi<T>*>(this)); }
};

template <typename T>
struct Arbi;

template <typename T>
struct ArbiContainer : public ArbiBase<T>
{
    ArbiContainer(size_t _minSize, size_t _maxSize) : minSize(_minSize), maxSize(_maxSize) {}

    Arbi<T>& setMinSize(size_t size)
    {
        minSize = size;
        return static_cast<Arbi<T>&>(*this);
    }

    Arbi<T>& setMaxSize(size_t size)
    {
        maxSize = size;
        return static_cast<Arbi<T>&>(*this);
    }

    Arbi<T>& setSize(size_t size)
    {
        minSize = size;
        maxSize = size;
        return static_cast<Arbi<T>&>(*this);
    }

    Arbi<T>& setSize(size_t min, size_t max)
    {
        minSize = min;
        maxSize = max;
        return static_cast<Arbi<T>&>(*this);
    }

    size_t minSize;
    size_t maxSize;
};

/**
 * @ingroup Generators
 * @brief \ref Arbitrary (alias) or Arbi is the default generator for basic types
 */
template <typename T>
struct Arbi : public ArbiBase<T>
{
};

/* Aliases */

/**
 * @ingroup Generators
 * @brief Arbitrary (alias for Arbi) is the default generator for basic types.
 */
template <typename... ARGS>
using Arbitrary = Arbi<ARGS...>;

template <typename... ARGS>
using ArbitraryBase = ArbiBase<ARGS...>;

template <typename... ARGS>
using ArbitraryContainer = ArbiContainer<ARGS...>;

#define DEFINE_ARBITRARY(TYPE, ...)                                                                         \
    template <>                                                                                             \
    struct Arbi<TYPE> : ArbiBase<TYPE>                                                                      \
    {                                                                                                       \
        ::proptest::Shrinkable<TYPE> operator()(::proptest::Random& rand) { return (__VA_ARGS__)()(rand); } \
    }

}  // namespace proptest
