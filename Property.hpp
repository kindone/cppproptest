#pragma once

#include "api.hpp"
#include "gen.hpp"
#include "util/function_traits.hpp"
#include "util/tuple.hpp"
#include "util/printing.hpp"
#include "util/invokeWithGenTuple.hpp"
#include "util/invokeWithArgs.hpp"
#include "util/createGenTuple.hpp"
#include "generator/util.hpp"
#include "PropertyContext.hpp"
#include "PropertyBase.hpp"
#include "Stream.hpp"
#include "util/std.hpp"

/**
 * @file Property.hpp
 * @brief Core API for `cppproptest` Property-based testing library
 */

namespace proptest {

namespace util {

struct Matrix
{
    template <size_t N, typename Lists>
    static decltype(auto) pickN(Lists&& lists, vector<int>& indices)
    {
        auto& vec = get<N>(lists);
        auto& index = indices[N];
        return vec[index];
    }

    template <size_t... index, typename Lists>
    static decltype(auto) pickEach(Lists&& lists, vector<int>& indices, index_sequence<index...>)
    {
        return util::make_tuple(pickN<index>(util::forward<Lists>(lists), indices)...);
    }

    template <size_t N, typename Lists>
    static decltype(auto) progressN(bool& incremented, Lists&& lists, vector<int>& indices)
    {
        auto& list = get<N>(lists);
        // already incremented
        if (incremented)
            return incremented;
        else if (indices[N] < static_cast<int>(list.size() - 1)) {
            indices[N]++;
            incremented = true;
        } else {
            incremented = false;
            indices[N] = 0;
        }
        return incremented;
    }

    template <size_t... index, typename Lists>
    static bool progress(Lists&& lists, vector<int>& indices, index_sequence<index...>)
    {
        constexpr auto Size = tuple_size<Lists>::value;
        bool incremented = false;
        [[maybe_unused]] std::initializer_list<int> runEach{
            (progressN<Size - index - 1>(incremented, util::forward<Lists>(lists), indices), 0)...};
        return incremented;
    }
};
}  // namespace util

/**
 * @brief Holder class for properties
 * @details When a property is defined using `proptest::property` or `proptest::forAll`, a `Property` object is created
 * to hold the property.
 */
template <typename... ARGS>
class Property final : public PropertyBase {
public:
    using Func = function<bool(ARGS...)>;
    using GenTuple = tuple<GenFunction<decay_t<ARGS>>...>;

private:
    using ArgTuple = tuple<decay_t<ARGS>...>;
    using ValueTuple = tuple<Shrinkable<decay_t<ARGS>>...>;
    using ShrinksTuple = tuple<conditional_t<is_same_v<ARGS, bool>, Stream, Stream>...>;

public:
    Property(const Func& f, const GenTuple& g) : PropertyBase(new Func(f), new GenTuple(g)) {}

private:
    bool invoke(Random& rand) { return util::invokeWithGenTuple(rand, getFunc(), getGenTup()); }

public:
    /**
     * @brief Sets the seed value for deterministic input generation
     *
     * @param s Seed in uint64_t type
     * @return Property& `Property` object itself for chaining
     */
    Property& setSeed(uint64_t s)
    {
        seed = s;
        return *this;
    }

    /**
     * @brief Sets the number of runs
     *
     * @param runs Number of runs
     * @return Property& `Property` object itself for chaining
     */
    Property& setNumRuns(uint32_t runs)
    {
        numRuns = runs;
        return *this;
    }

    /**
     * @brief Sets the startup function
     *
     * @param onStartup Invoked in each run before running the property function
     * @return Property& `Property` object itself for chaining
     */
    Property& setOnStartup(function<void()> onStartup)
    {
        onStartupPtr = util::make_shared<function<void()>>(onStartup);
        return *this;
    }

    /**
     * @brief Sets the cleanup function
     *
     * @param onCleanup Invoked in each run after running the property function.
     * @return Property& `Property` object itself for chaining
     */
    Property& setOnCleanup(function<void()> onCleanup)
    {
        onCleanupPtr = util::make_shared<function<void()>>(onCleanup);
        return *this;
    }

    /**
     * @brief Sets the timeout duration.
     *
     * @param durationMs maximum time to spend in the main loop, in milliseconds. Default is 0 meaning there is no timeout.
     * @return Property& `Property` object itself for chaining
     */
    Property& setMaxDurationMs(uint32_t durationMs)
    {
        maxDurationMs = durationMs;
        return *this;
    }

    /**
     * @brief Executes randomized tests for given property. If explicit generator arguments are omitted, utilizes
     * default generators (a.k.a. Arbitraries) instead
     *
     * Usage of explicit generators:
     * @code
     *  auto prop = property([](int a, float b) { ... });
     *  prop.forAll();                  // Arbitrary<int>, Arbitrary<float> is used to generate a and b
     *  prop.forAll(intGen);            // intGen, Arbitrary<float> is used to generate a and b
     *  prop.forAll(intGen, floatGen);  // intGen, floatGen is used to generate a and b
     * @endcode
     * @tparam ExplicitGens Explicitly given generator types
     * @param gens Variadic list of optional explicit generators (in same order as in definition of property arguments)
     * @return true if all the cases succeed
     * @return false if any one of the cases fails
     */
    template <typename... ExplicitGens>
    bool forAll(ExplicitGens&&... gens)
    {
        // combine explicit generators and implicit generators into a tuple by overriding implicit generators with explicit generators
        auto curGenTup = util::overrideTuple(getGenTup(), gens...);
        return runForAll(util::forward<decltype(curGenTup)>(curGenTup));
    }

    /**
     * @brief Executes single example-based test for given property.
     *
     * @param args Variadic list of explicit arguments (in same order as in definition of property arguments)
     * @return true if the case succeeds
     * @return false if the cases fails
     */
    bool example(ARGS&&... args)
    {
        auto valueTup = util::make_tuple(args...);
        return example(valueTup);
    }

private:

    bool runForAll(GenTuple&& curGenTup)
    {
        Random rand(seed);
        Random savedRand(seed);
        cout << "random seed: " << seed << endl;
        PropertyContext ctx;
        auto startedTime = steady_clock::now();

        size_t i = 0;
        try {
            for (; i < numRuns; i++) {
                if(maxDurationMs != 0) {
                    auto currentTime = steady_clock::now();
                    if(duration_cast<util::milliseconds>(currentTime - startedTime).count() > maxDurationMs)
                    {
                        cout << "Timed out after " << duration_cast<util::milliseconds>(currentTime - startedTime).count() << "ms , passed " << i << " tests" << endl;
                        ctx.printSummary();
                        return true;
                    }
                }
                bool pass = true;
                do {
                    pass = true;
                    try {
                        savedRand = rand;
                        if (onStartupPtr)
                            (*onStartupPtr)();
                        bool result = util::invokeWithGenTuple(rand, getFunc(), curGenTup);
                        if (onCleanupPtr)
                            (*onCleanupPtr)();
                        stringstream failures = ctx.flushFailures();
                        // failed expectations
                        if (failures.rdbuf()->in_avail()) {
                            cerr << "Falsifiable, after " << (i + 1) << " tests: ";
                            cerr << failures.str();
                            shrink(savedRand, util::forward<GenTuple>(curGenTup));
                            return false;
                        } else if (!result) {
                            cerr << "Falsifiable, after " << (i + 1) << " tests" << endl;
                            shrink(savedRand, util::forward<GenTuple>(curGenTup));
                            return false;
                        }
                        pass = true;
                    } catch (const Success&) {
                        pass = true;
                    } catch (const Discard&) {
                        // silently discard combination
                        pass = false;
                    }
                } while (!pass);
            }
        } catch (const AssertFailed& e) {
            cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":" << e.lineno
                 << ")" << endl;
            // shrink
            shrink(savedRand, util::forward<GenTuple>(curGenTup));
            return false;
        } catch (const PropertyFailedBase& e) {
            cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":" << e.lineno
                 << ")" << endl;
            // shrink
            shrink(savedRand, util::forward<GenTuple>(curGenTup));
            return false;
        } catch (const exception& e) {
            cerr << "Falsifiable, after " << (i + 1) << " tests - unhandled exception thrown: " << e.what() << endl;
            // shrink
            shrink(savedRand, util::forward<GenTuple>(curGenTup));
            return false;
        }

        cout << "OK, passed " << numRuns << " tests" << endl;
        ctx.printSummary();
        return true;
    }

    bool example(const tuple<ARGS...>& valueTup)
    {
        PropertyContext context;
        try {
            try {
                try {
                    if (onStartupPtr)
                        (*onStartupPtr)();
                    bool result = util::invokeWithArgs(getFunc(), valueTup);
                    if (onCleanupPtr)
                        (*onCleanupPtr)();
                    return result;
                } catch (const AssertFailed& e) {
                    throw PropertyFailed<tuple<ARGS...>>(e);
                }
            } catch (const Success&) {
                return true;
            } catch (const Discard&) {
                // silently discard combination
                cerr << "Discard is not supported for single run" << endl;
            }
        } catch (const PropertyFailedBase& e) {
            cerr << "example failed: " << e.what() << " (" << e.filename << ":" << e.lineno << ")" << endl;
            cerr << "  with args: " << Show<tuple<ARGS...>>(valueTup) << endl;
            return false;
        } catch (const exception& e) {
            // skip shrinking?
            cerr << "example failed by exception: " << e.what() << endl;
            cerr << "  with args: " << Show<tuple<ARGS...>>(valueTup) << endl;
            return false;
        }
        return false;
    }

public:
    /**
    * @brief Executes all input combinations in the Cartesian product of input lists
    *
    * Usage:
    * @code
        // As property is defined with two arguments: int and float, matrix() requires two arguments: int and float
    lists
        auto prop = property([](int i, float f) {
            // ...
        });

        // examples are auto-generated as Cartesian propduct of the lists:
        //   {1, 0.2f}, {1, 0.3f}, {2, 0.2f}, {2, 0.3f}, {3, 0.2f}, {3, 0.3f}
        prop.matrix({1,2,3}, {0.2f, 0.3f});
    * @endcode
    * @param lists Lists of valid arguments (types must be in same order as in parameters of the callable)
    */
    bool matrix(initializer_list<ARGS>&&... lists)
    {
        constexpr auto Size = sizeof...(ARGS);
        auto vecTuple = util::make_tuple(vector<ARGS>(lists)...);
        vector<int> indices;
        for (size_t i = 0; i < Size; i++)
            indices.push_back(0);

        do {
            auto valueTup = util::Matrix::pickEach(util::forward<decltype(vecTuple)>(vecTuple), indices,
                                                   make_index_sequence<Size>{});
            example(valueTup);
        } while (
            util::Matrix::progress(util::forward<decltype(vecTuple)>(vecTuple), indices, make_index_sequence<Size>{}));

        return true;
    }

private:
    template <typename Invoker, typename Replace>
    bool test(Invoker invoker, ValueTuple&& valueTup, Replace&& replace)
    {
        bool result = false;
        auto values = util::transformHeteroTuple<util::ShrinkableGet>(util::forward<ValueTuple>(valueTup));
        try {
            if (onStartupPtr)
                (*onStartupPtr)();
            result = invoker(getFunc(), util::forward<decltype(values)>(values), replace.get());
            if (onCleanupPtr)
                (*onCleanupPtr)();
        } catch (const AssertFailed&) {
            result = false;
            // cerr << "    assertion failed: " << e.what() << " (" << e.filename << ":"
            //           << e.lineno << ")" << endl;
        } catch (const exception&) {
            result = false;
        }
        return result;
    }

    template <typename Shrinks>
    static void printShrinks(const Shrinks& shrinks)
    {
        auto itr = shrinks.iterator();
        // cout << "    shrinks: " << endl;
        for (int i = 0; i < 4 && itr.hasNext(); i++) {
            auto& value = itr.next();
            cout << "    " << Show<decay_t<decltype(value)>>(value) << endl;
        }
    }

    template <size_t N, typename ValueTuple, typename ShrinksTuple>
    decltype(auto) shrinkN(ValueTuple&& valueTup, ShrinksTuple&& shrinksTuple)
    {
        using ShrinksType = tuple_element_t<N, ValueTuple>;//tuple_element_t<N, tuple<Shrinkable<decay_t<ARGS>>...>>;
        auto shrinks = get<N>(shrinksTuple);
        // keep shrinking until no shrinking is possible
        while (!shrinks.isEmpty()) {
            // printShrinks(shrinks);
            auto iter = shrinks.template iterator<ShrinksType>();
            bool shrinkFound = false;
            PropertyContext context;
            // keep trying until failure is reproduced
            while (iter.hasNext()) {
                // get shrinkable
                auto next = iter.next();
                if (!test(util::invokeWithArgTupleWithReplace<N, Func&, ArgTuple, typename decltype(next)::type>,
                          util::forward<ValueTuple>(valueTup), next) ||
                    context.hasFailures()) {
                    shrinks = next.shrinks();
                    get<N>(valueTup) = next;
                    shrinkFound = true;
                    break;
                }
            }
            if (shrinkFound) {
                cout << "  shrinking found simpler failing arg " << N << ": " << Show<ValueTuple>(valueTup) << endl;
                if (context.hasFailures())
                    cout << "    by failed expectation: " << context.flushFailures(4).str() << endl;
            } else {
                break;
            }
        }
        // cout << "  no more shrinking found for arg " << N << endl;
        return get<N>(valueTup);
    }

    template <size_t... index, typename ValueTuple, typename ShrinksTuple>
    decltype(auto) shrinkEach(ValueTuple&& valueTup, ShrinksTuple&& shrinksTup, index_sequence<index...>)
    {
        return util::make_tuple(
            shrinkN<index>(util::forward<ValueTuple>(valueTup), util::forward<ShrinksTuple>(shrinksTup))...);
    }

    void shrink(Random& savedRand, GenTuple&& curGenTup)
    {
        // regenerate failed value tuple
        auto generatedValueTup =
            util::transformHeteroTupleWithArg<util::Generate>(util::forward<GenTuple>(curGenTup), savedRand);

        cout << "  with args: " << Show<decltype(generatedValueTup)>(generatedValueTup) << endl;
        // cout << (valueTup == valueTup2 ? "gen equals original" : "gen not equals original") << endl;
        static constexpr auto Size = tuple_size<GenTuple>::value;
        auto shrinksTuple =
            util::transformHeteroTuple<util::GetShrinks>(util::forward<decltype(generatedValueTup)>(generatedValueTup));
        auto shrunk = shrinkEach(util::forward<decltype(generatedValueTup)>(generatedValueTup),
                                 util::forward<decltype(shrinksTuple)>(shrinksTuple), make_index_sequence<Size>{});
        cout << "  simplest args found by shrinking: " << Show<decltype(shrunk)>(shrunk) << endl;
    }

    Func& getFunc() { return *static_pointer_cast<Func>(funcPtr); }

    GenTuple& getGenTup() { return *static_pointer_cast<GenTuple>(genTupPtr); }
};

namespace util {

template <typename RetType, typename Callable, typename... ARGS>
enable_if_t<is_same<RetType, bool>::value, function<bool(ARGS...)>> functionWithBoolResultHelper(
    util::TypeList<ARGS...>, Callable&& callable)
{
    return static_cast<function<RetType(ARGS...)>>(callable);
}

template <typename RetType, typename Callable, typename... ARGS>
enable_if_t<is_same<RetType, void>::value, function<bool(ARGS...)>> functionWithBoolResultHelper(
    util::TypeList<ARGS...>, Callable&& callable)
{
    return function<bool(ARGS...)>([callable](ARGS&&... args) {
        callable(util::forward<ARGS>(args)...);
        return true;
    });
}

template <class Callable>
decltype(auto) functionWithBoolResult(Callable&& callable)
{
    using RetType = typename function_traits<Callable>::return_type;
    typename function_traits<Callable>::argument_type_list argument_type_list;
    return functionWithBoolResultHelper<RetType>(argument_type_list, util::forward<Callable>(callable));
}

template <typename RetType, typename Callable, typename... ARGS>
function<RetType(ARGS...)> asFunctionHelper(util::TypeList<ARGS...>, Callable&& callable)
{
    return static_cast<function<RetType(ARGS...)>>(callable);
}

template <class Callable>
decltype(auto) asFunction(Callable&& callable)
{
    using RetType = typename function_traits<Callable>::return_type;
    typename function_traits<Callable>::argument_type_list argument_type_list;
    return asFunctionHelper<RetType>(argument_type_list, util::forward<Callable>(callable));
}

template <typename... ARGS>
decltype(auto) createProperty(function<bool(ARGS...)> func, tuple<GenFunction<decay_t<ARGS>>...>&& genTup)
{
    return Property<ARGS...>(func, genTup);
}

}  // namespace util

/**
 * @brief creates a property object that can be used to run various property tests
 * @details @see Property
 * @tparam Callable property callable type in either `(ARGS...) -> bool` (success/fail by boolean return value) or
 * `(ARGS...) -> void` (fail if exception is thrown, success eitherwise)
 * @tparam ExplicitGens Explicit generator callable types for each `ARG` in `(Random&) -> Shrinkable<ARG>`
 * @param callable passed as any either `std::function`, functor object, function pointer
 * @param gens variadic list of generators for `ARG`s (optional if `Arbitrary<ARG>` is preferred)
 */
template <typename Callable, typename... ExplicitGens>
auto property(Callable&& callable, ExplicitGens&&... gens)
{
    // acquire full tuple of generators
    typename function_traits<Callable>::argument_type_list argument_type_list;
    auto func = util::functionWithBoolResult(callable);
    auto genTup = util::createGenTuple(argument_type_list, util::asFunction(util::forward<decltype(gens)>(gens))...);
    return util::createProperty(func, util::forward<decltype(genTup)>(genTup));
}
/**
 * @brief Immediately executes a randomized property test
 *
 * equivalent to `property(...).forAll()`
 *
 * @tparam Callable property callable type in either `(ARGS...) -> bool` (success/fail by boolean return value) or
 * `(ARGS...) -> void` (fail if exception is thrown, success eitherwise)
 * @tparam ExplicitGens Explicit generator callable types for `ARG` in `(Random&) -> Shrinkable<ARG>`
 * @param callable passed as any either `std::function`, functor object, function pointer
 * @param gens variadic list of generators for `ARG`s (optional if `Arbitrary<ARG>` is preferred)
 * @return true if all the cases succeed
 * @return false if any one of the cases fails
 */
template <typename Callable, typename... ExplicitGens>
bool forAll(Callable&& callable, ExplicitGens&&... gens)
{
    return property(callable, gens...).forAll();
}

/**
* @brief Immediately executes all input combinations in the Cartesian product of input lists
*
* equivalent to `property(...).forAll()`
*
* Usage:
* @code
    // If property callable is defined with two arguments: int and float,
    // matrix() requires as arguments the callable and two more of types initializer_list<int> and initializer_list<float>.
lists
    // examples are auto-generated as Cartesian propduct of the lists:
    //   {1, 0.2f}, {1, 0.3f}, {2, 0.2f}, {2, 0.3f}, {3, 0.2f}, {3, 0.3f}
    proptest::matrix([](int i, float f) {
        // ...
    }, {1,2,3}, {0.2f, 0.3f});
* @endcode
*
* @tparam Callable property callable type in either `(ARGS...) -> bool` (success/fail by boolean return value) or
* `(ARGS...) -> void` (fail if exception is thrown, success eitherwise)
* @tparam ARGS variadic types for callable and the initializer_lists
* @param callable passed as any either `std::function`, functor object, function pointer
* @param lists Lists of valid arguments (types must be in same order as in parameters of the callable)
* @return true if all the cases succeed
* @return false if any one of the cases fails
*/

template <typename Callable, typename... ARGS>
bool matrix(Callable&& callable, initializer_list<ARGS>&&... lists)
{
    return property(callable).matrix(util::forward<decltype(lists)>(lists)...);
}

#define EXPECT_FOR_ALL(CALLABLE, ...) EXPECT_TRUE(proptest::forAll(CALLABLE, __VA_ARGS__))
#define ASSERT_FOR_ALL(CALLABLE, ...) ASSERT_TRUE(proptest::forAll(CALLABLE, __VA_ARGS__))

}  // namespace proptest
