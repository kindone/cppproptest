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
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <initializer_list>

namespace proptest {

template <typename... ARGS>
struct Matrix {
    template <size_t N>
    static decltype(auto) pickN(std::tuple<std::vector<ARGS>...>&& lists, std::vector<int>& indices) {
        auto& vec = std::get<N>(lists);
        auto& index = indices[N];
        return vec[index];
    }

    template <size_t... index>
    static decltype(auto) pickEach(std::tuple<std::vector<ARGS>...>&& lists, std::vector<int>& indices, std::index_sequence<index...>)
    {
        return std::make_tuple(
            pickN<index>(std::forward<std::tuple<std::vector<ARGS>...>>(lists), indices)...);
    }

    template <size_t N>
    static decltype(auto) progressN(bool& incremented, std::tuple<std::vector<ARGS>...>&& lists, std::vector<int>& indices) {
        auto& list = std::get<N>(lists);
        // already incremented
        if(incremented)
            return incremented;
        else if(indices[N] < list.size()-1) {
            indices[N] ++;
            incremented = true;
        }
        else {
            incremented = false;
            indices[N] = 0;
        }
        return incremented;
    }

    template <size_t... index>
    static bool progress(std::tuple<std::vector<ARGS>...>&& lists, std::vector<int>& indices, std::index_sequence<index...>)
    {
        constexpr auto Size = sizeof...(ARGS);
        bool incremented = false;
        std::make_tuple(progressN<Size-index-1>(incremented, std::forward<decltype(lists)>(lists), indices)...);
        return incremented;
    }
};

template <typename... ARGS>
class Property final : public PropertyBase {
public:
    using Func = std::function<bool(ARGS...)>;
    using GenTuple = std::tuple<GenFunction<std::decay_t<ARGS>>...>;
    using ValueTuple = std::tuple<Shrinkable<std::decay_t<ARGS>>...>;
    using ShrinksTuple = std::tuple<Stream<Shrinkable<std::decay_t<ARGS>>>...>;

    Property(const Func& f, const GenTuple& g) : func(f), genTup(g) {}

    virtual bool invoke(Random& rand) override { return util::invokeWithGenTuple(rand, func, genTup); }

    Property& setSeed(uint64_t s)
    {
        seed = s;
        return *this;
    }

    Property& setNumRuns(uint32_t runs)
    {
        numRuns = runs;
        return *this;
    }

    Property& setOnStartup(std::function<void()> onStartup) {
        onStartupPtr = std::make_shared<std::function<void()>>(onStartup);
        return *this;
    }

    Property& setOnCleanup(std::function<void()> onCleanup) {
        onCleanupPtr = std::make_shared<std::function<void()>>(onCleanup);
        return *this;
    }

    template <typename... EXPGENS>
    bool forAll(EXPGENS&&... gens)
    {
        Random rand(seed);
        Random savedRand(seed);
        std::cout << "random seed: " << seed << std::endl;
        PropertyContext ctx;
        auto curGenTup = util::overrideTuple(genTup, gens...);

        int i = 0;
        try {
            for (; i < numRuns; i++) {
                bool pass = true;
                do {
                    pass = true;
                    try {
                        savedRand = rand;
                        if(onStartupPtr)
                            (*onStartupPtr)();
                        bool result = util::invokeWithGenTuple(rand, func, curGenTup);
                        if(onCleanupPtr)
                            (*onCleanupPtr)();
                        std::stringstream failures = ctx.flushFailures();
                        // failed expectations
                        if (failures.rdbuf()->in_avail()) {
                            std::cerr << "Falsifiable, after " << (i + 1) << " tests: ";
                            std::cerr << failures.str();
                            shrink(savedRand, std::forward<GenTuple>(curGenTup));
                            return false;
                        } else if (!result) {
                            std::cerr << "Falsifiable, after " << (i + 1) << " tests" << std::endl;
                            shrink(savedRand, std::forward<GenTuple>(curGenTup));
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
            std::cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":"
                      << e.lineno << ")" << std::endl;
            // shrink
            shrink(savedRand, std::forward<GenTuple>(curGenTup));
            return false;
        } catch (const PropertyFailedBase& e) {
            std::cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":"
                      << e.lineno << ")" << std::endl;
            // shrink
            shrink(savedRand, std::forward<GenTuple>(curGenTup));
            return false;
        } catch (const std::exception& e) {
            std::cerr << "Falsifiable, after " << (i + 1) << " tests - unhandled exception thrown: " << e.what()
                      << std::endl;
            // shrink
            shrink(savedRand, std::forward<GenTuple>(curGenTup));
            return false;
        }

        std::cout << "OK, passed " << numRuns << " tests" << std::endl;
        ctx.printSummary();
        return true;
    }

    bool example(ARGS&&... args)
    {
        auto valueTup = std::make_tuple(args...);
        return example(valueTup);
    }

    bool example(const std::tuple<ARGS...>& valueTup)
    {
        PropertyContext context;
        try {
            try {
                try {
                    if(onStartupPtr)
                        (*onStartupPtr)();
                    bool result = util::invokeWithArgs(func, valueTup);
                    if(onCleanupPtr)
                        (*onCleanupPtr)();
                    return result;
                } catch (const AssertFailed& e) {
                    throw PropertyFailed<std::tuple<ARGS...>>(e);
                }
            } catch (const Success&) {
                return true;
            } catch (const Discard&) {
                // silently discard combination
                std::cerr << "Discard is not supported for single run" << std::endl;
            }
        } catch (const PropertyFailedBase& e) {
            std::cerr << "example failed: " << e.what() << " (" << e.filename << ":" << e.lineno << ")" << std::endl;
            std::cerr << "  with args: " << Show<std::tuple<ARGS...>>(valueTup) << std::endl;
            return false;
        } catch (const std::exception& e) {
            // skip shrinking?
            std::cerr << "example failed by std::exception: " << e.what() << std::endl;
            std::cerr << "  with args: " << Show<std::tuple<ARGS...>>(valueTup) << std::endl;
            return false;
        }
        return false;
    }

    /* TODO: Test all input combinations in the Cartesian product of input lists
    */
    bool matrix(std::initializer_list<ARGS>&&...lists)
    {
        constexpr auto Size = sizeof...(ARGS);
        auto vecTuple = std::make_tuple(std::vector<ARGS>(lists)...);
        std::vector<int> indices;
        for(size_t i = 0; i < Size; i++)
            indices.push_back(0);

        do {
            [[maybe_unused]] auto valueTup = Matrix<ARGS...>::pickEach(std::forward<decltype(vecTuple)>(vecTuple), indices, std::make_index_sequence<Size>{});
            example(valueTup);
        } while((Matrix<ARGS...>::progress(std::forward<decltype(vecTuple)>(vecTuple), indices, std::make_index_sequence<Size>{})));

        return true;
    }

private:
    template <size_t N, typename Replace>
    bool test(ValueTuple&& valueTup, Replace&& replace)
    {
        bool result = false;
        auto values = util::transformHeteroTuple<util::ShrinkableGet>(std::forward<ValueTuple>(valueTup));
        try {
            if(onStartupPtr)
                (*onStartupPtr)();
            result =
                util::invokeWithArgTupleWithReplace<N>(func, std::forward<decltype(values)>(values), replace.get());
            if(onCleanupPtr)
                (*onCleanupPtr)();
        } catch (const AssertFailed&) {
            result = false;
            // std::cerr << "    assertion failed: " << e.what() << " (" << e.filename << ":"
            //           << e.lineno << ")" << std::endl;
        } catch (const std::exception&) {
            result = false;
        }
        return result;
    }

    template <typename Shrinks>
    static void printShrinks(const Shrinks& shrinks)
    {
        auto itr = shrinks.iterator();
        // std::cout << "    shrinks: " << std::endl;
        for (int i = 0; i < 4 && itr.hasNext(); i++) {
            auto& value = itr.next();
            std::cout << "    " << Show<std::decay_t<decltype(value)>>(value) << std::endl;
        }
    }

    template <size_t N>
    decltype(auto) shrinkN(ValueTuple&& valueTup, ShrinksTuple&& shrinksTuple)
    {
        auto shrinks = std::get<N>(shrinksTuple);
        // keep shrinking until no shrinking is possible
        while (!shrinks.isEmpty()) {
            // printShrinks(shrinks);
            auto iter = shrinks.iterator();
            bool shrinkFound = false;
            PropertyContext context;
            // keep trying until failure is reproduced
            while (iter.hasNext()) {
                // get shrinkable
                auto next = iter.next();
                if (!test<N>(std::forward<ValueTuple>(valueTup), next) || context.hasFailures()) {
                    shrinks = next.shrinks();
                    std::get<N>(valueTup) = next;
                    shrinkFound = true;
                    break;
                }
            }
            if (shrinkFound) {
                std::cout << "  shrinking found simpler failing arg " << N << ": " << Show<ValueTuple>(valueTup)
                          << std::endl;
                if (context.hasFailures())
                    std::cout << "    by failed expectation: " << context.flushFailures(4).str() << std::endl;
            } else {
                break;
            }
        }
        // std::cout << "  no more shrinking found for arg " << N << std::endl;
        return std::get<N>(valueTup);
    }

    template <size_t... index>
    decltype(auto) shrinkEach(ValueTuple&& valueTup, ShrinksTuple&& shrinksTup, std::index_sequence<index...>)
    {
        return std::make_tuple(
            shrinkN<index>(std::forward<ValueTuple>(valueTup), std::forward<ShrinksTuple>(shrinksTup))...);
    }

    void shrink(Random& savedRand, GenTuple&& curGenTup)
    {
        // regenerate failed value tuple
        auto generatedValueTup =
            util::transformHeteroTupleWithArg<util::Generate>(std::forward<GenTuple>(curGenTup), savedRand);

        std::cout << "  with args: " << Show<decltype(generatedValueTup)>(generatedValueTup) << std::endl;
        // std::cout << (valueTup == valueTup2 ? "gen equals original" : "gen not equals original") << std::endl;
        static constexpr auto Size = std::tuple_size<GenTuple>::value;
        auto shrinksTuple =
            util::transformHeteroTuple<util::GetShrinks>(std::forward<decltype(generatedValueTup)>(generatedValueTup));
        auto shrunk = shrinkEach(std::forward<decltype(generatedValueTup)>(generatedValueTup),
                                 std::forward<decltype(shrinksTuple)>(shrinksTuple), std::make_index_sequence<Size>{});
        std::cout << "  simplest args found by shrinking: " << Show<decltype(shrunk)>(shrunk) << std::endl;
    }

private:
    Func func;
    GenTuple genTup;
    std::shared_ptr<std::function<void()>> onStartupPtr;
    std::shared_ptr<std::function<void()>> onCleanupPtr;
};

namespace util {

template <typename RetType, typename Callable, typename... ARGS>
std::enable_if_t<std::is_same<RetType, bool>::value, std::function<bool(ARGS...)>> functionWithBoolResultHelper(
    util::TypeList<ARGS...>, Callable&& callable)
{
    return static_cast<std::function<RetType(ARGS...)>>(callable);
}

template <typename RetType, typename Callable, typename... ARGS>
std::enable_if_t<std::is_same<RetType, void>::value, std::function<bool(ARGS...)>> functionWithBoolResultHelper(
    util::TypeList<ARGS...>, Callable&& callable)
{
    return std::function<bool(ARGS...)>([callable](ARGS&&... args) {
        callable(std::forward<ARGS>(args)...);
        return true;
    });
}

template <class Callable>
decltype(auto) functionWithBoolResult(Callable&& callable)
{
    using RetType = typename function_traits<Callable>::return_type;
    typename function_traits<Callable>::argument_type_list argument_type_list;
    return functionWithBoolResultHelper<RetType>(argument_type_list, std::forward<Callable>(callable));
}

template <typename RetType, typename Callable, typename... ARGS>
std::function<RetType(ARGS...)> asFunctionHelper(util::TypeList<ARGS...>, Callable&& callable)
{
    return static_cast<std::function<RetType(ARGS...)>>(callable);
}

template <class Callable>
decltype(auto) asFunction(Callable&& callable)
{
    using RetType = typename function_traits<Callable>::return_type;
    typename function_traits<Callable>::argument_type_list argument_type_list;
    return asFunctionHelper<RetType>(argument_type_list, std::forward<Callable>(callable));
}

template <typename... ARGS>
decltype(auto) createProperty(std::function<bool(ARGS...)> func,
                              std::tuple<GenFunction<std::decay_t<ARGS>>...>&& genTup)
{
    return Property<ARGS...>(func, genTup);
}

}  // namespace util

template <typename Callable, typename... EXPGENS>
auto property(Callable&& callable, EXPGENS&&... gens)
{
    // acquire full tuple of generators
    typename function_traits<Callable>::argument_type_list argument_type_list;
    auto func = util::functionWithBoolResult(callable);
    auto genTup = util::createGenTuple(argument_type_list, util::asFunction(std::forward<decltype(gens)>(gens))...);
    return util::createProperty(func, std::forward<decltype(genTup)>(genTup));
}

template <typename Callable, typename... EXPGENS>
bool forAll(Callable&& callable, EXPGENS&&... gens)
{
    return property(callable, gens...).forAll();
}

#define EXPECT_FOR_ALL(CALLABLE, ...) EXPECT_TRUE(proptest::forAll(CALLABLE, __VA_ARGS__))
#define ASSERT_FOR_ALL(CALLABLE, ...) ASSERT_TRUE(proptest::forAll(CALLABLE, __VA_ARGS__))

}  // namespace proptest
