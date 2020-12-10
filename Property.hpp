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

namespace proptest {

namespace util {
struct Matrix {
    template <size_t N, typename Lists>
    static decltype(auto) pickN(Lists&& lists, vector<int>& indices) {
        auto& vec = get<N>(lists);
        auto& index = indices[N];
        return vec[index];
    }

    template <size_t... index, typename Lists>
    static decltype(auto) pickEach(Lists&& lists, vector<int>& indices, index_sequence<index...>)
    {
        return util::make_tuple(
            pickN<index>(util::forward<Lists>(lists), indices)...);
    }

    template <size_t N, typename Lists>
    static decltype(auto) progressN(bool& incremented, Lists&& lists, vector<int>& indices) {
        auto& list = get<N>(lists);
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

    template <size_t... index, typename Lists>
    static bool progress(Lists&& lists, vector<int>& indices, index_sequence<index...>)
    {
        constexpr auto Size = tuple_size<Lists>::value;
        bool incremented = false;
        util::make_tuple(progressN<Size-index-1>(incremented, util::forward<Lists>(lists), indices)...);
        return incremented;
    }
};
} // namespace util

template <typename... ARGS>
class Property final : public PropertyBase {
public:
    using Func = function<bool(ARGS...)>;
    using GenTuple = tuple<GenFunction<decay_t<ARGS>>...>;
    using ValueTuple = tuple<Shrinkable<decay_t<ARGS>>...>;
    using ShrinksTuple = tuple<Stream<Shrinkable<decay_t<ARGS>>>...>;

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

    Property& setOnStartup(function<void()> onStartup) {
        onStartupPtr = util::make_shared<function<void()>>(onStartup);
        return *this;
    }

    Property& setOnCleanup(function<void()> onCleanup) {
        onCleanupPtr = util::make_shared<function<void()>>(onCleanup);
        return *this;
    }

    template <typename... EXPGENS>
    bool forAll(EXPGENS&&... gens)
    {
        Random rand(seed);
        Random savedRand(seed);
        cout << "random seed: " << seed << endl;
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
            cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":"
                      << e.lineno << ")" << endl;
            // shrink
            shrink(savedRand, util::forward<GenTuple>(curGenTup));
            return false;
        } catch (const PropertyFailedBase& e) {
            cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":"
                      << e.lineno << ")" << endl;
            // shrink
            shrink(savedRand, util::forward<GenTuple>(curGenTup));
            return false;
        } catch (const exception& e) {
            cerr << "Falsifiable, after " << (i + 1) << " tests - unhandled exception thrown: " << e.what()
                      << endl;
            // shrink
            shrink(savedRand, util::forward<GenTuple>(curGenTup));
            return false;
        }

        cout << "OK, passed " << numRuns << " tests" << endl;
        ctx.printSummary();
        return true;
    }

    bool example(ARGS&&... args)
    {
        auto valueTup = util::make_tuple(args...);
        return example(valueTup);
    }

    bool example(const tuple<ARGS...>& valueTup)
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

    /* TODO: Test all input combinations in the Cartesian product of input lists
    */
    bool matrix(initializer_list<ARGS>&&...lists)
    {
        constexpr auto Size = sizeof...(ARGS);
        auto vecTuple = util::make_tuple(vector<ARGS>(lists)...);
        vector<int> indices;
        for(size_t i = 0; i < Size; i++)
            indices.push_back(0);

        do {
            auto valueTup = util::Matrix::pickEach(util::forward<decltype(vecTuple)>(vecTuple), indices, make_index_sequence<Size>{});
            example(valueTup);
        } while(util::Matrix::progress(util::forward<decltype(vecTuple)>(vecTuple), indices, make_index_sequence<Size>{}));

        return true;
    }

private:
    template <size_t N, typename Replace>
    bool test(ValueTuple&& valueTup, Replace&& replace)
    {
        bool result = false;
        auto values = util::transformHeteroTuple<util::ShrinkableGet>(util::forward<ValueTuple>(valueTup));
        try {
            if(onStartupPtr)
                (*onStartupPtr)();
            result =
                util::invokeWithArgTupleWithReplace<N>(func, util::forward<decltype(values)>(values), replace.get());
            if(onCleanupPtr)
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
        auto shrinks = get<N>(shrinksTuple);
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
                if (!test<N>(util::forward<ValueTuple>(valueTup), next) || context.hasFailures()) {
                    shrinks = next.shrinks();
                    get<N>(valueTup) = next;
                    shrinkFound = true;
                    break;
                }
            }
            if (shrinkFound) {
                cout << "  shrinking found simpler failing arg " << N << ": " << Show<ValueTuple>(valueTup)
                          << endl;
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

private:
    Func func;
    GenTuple genTup;
    shared_ptr<function<void()>> onStartupPtr;
    shared_ptr<function<void()>> onCleanupPtr;
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
decltype(auto) createProperty(function<bool(ARGS...)> func,
                              tuple<GenFunction<decay_t<ARGS>>...>&& genTup)
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
    auto genTup = util::createGenTuple(argument_type_list, util::asFunction(util::forward<decltype(gens)>(gens))...);
    return util::createProperty(func, util::forward<decltype(genTup)>(genTup));
}

template <typename Callable, typename... EXPGENS>
bool forAll(Callable&& callable, EXPGENS&&... gens)
{
    return property(callable, gens...).forAll();
}

#define EXPECT_FOR_ALL(CALLABLE, ...) EXPECT_TRUE(proptest::forAll(CALLABLE, __VA_ARGS__))
#define ASSERT_FOR_ALL(CALLABLE, ...) ASSERT_TRUE(proptest::forAll(CALLABLE, __VA_ARGS__))

}  // namespace proptest
