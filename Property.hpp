#pragma once

#include "api.hpp"
#include "gen.hpp"
#include "util/function_traits.hpp"
#include "util/tuple.hpp"
#include "util/printing.hpp"
#include "generator/util.hpp"
#include "PropertyContext.hpp"
#include "PropertyBase.hpp"
#include "Stream.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>

namespace PropertyBasedTesting {
namespace util {

template <typename T>
decltype(auto) ReturnTypeOf()
{
    TypeHolder<typename std::result_of<decltype (&T::operator())(T, Random&)>::type> typeHolder;
    return typeHolder;
}

template <typename... ARGS>
decltype(auto) ReturnTypeTupleFromGenTup(std::tuple<ARGS...>& tup)
{
    TypeList<typename decltype(ReturnTypeOf<ARGS>())::type...> typeList;
    return typeList;
}

}  // namespace util

template <typename... ARGS>
class Property final : public PropertyBase {
public:
    using Func = std::function<bool(ARGS...)>;
    using GenTuple = std::tuple<std::function<Shrinkable<std::decay_t<ARGS>>(Random&)>...>;
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

    bool example(ARGS&&... args)
    {
        PropertyContext context;
        auto valueTup = std::make_tuple(args...);
        auto valueTupPtr = std::make_shared<decltype(valueTup)>(valueTup);
        try {
            try {
                try {
                    return util::invokeWithArgs(func, std::forward<ARGS>(args)...);
                } catch (const AssertFailed& e) {
                    throw PropertyFailed<decltype(valueTup)>(e, valueTupPtr);
                }
            } catch (const Success&) {
                return true;
            } catch (const Discard&) {
                // silently discard combination
                std::cerr << "Discard is not supported for single run" << std::endl;
            }
        } catch (const PropertyFailedBase& e) {
            std::cerr << "Property failed: " << e.what() << " (" << e.filename << ":" << e.lineno << ")" << std::endl;
            std::cout << "  with args: ";
            show(std::cout, valueTup);
            std::cout << std::endl;
            return false;
        } catch (const std::exception& e) {
            // skip shrinking?
            std::cerr << "std::exception occurred: " << e.what() << std::endl;
            return false;
        }
        return false;
    }

    virtual void handleShrink(Random& savedRand /*, const PropertyFailedBase& e*/) override
    {
        // auto retTypeTup = util::ReturnTypeTupleFromGenTup(genTup);
        // using ValueTuple = typename decltype(retTypeTup)::type_tuple;

        // auto failed = dynamic_cast<const PropertyFailed<ValueTuple>&>(e);
        shrink(savedRand /*, *failed.valueTupPtr*/);
    }

private:
    template <size_t N, typename Replace>
    bool test(ValueTuple&& valueTup, Replace&& replace)
    {
        // std::cout << "    test: tuple ";
        // show(std::cout, valueTup);
        // std::cout << " replaced with arg " << N << ": ";
        // show(std::cout, replace);
        // std::cout << std::endl;
        bool result = false;
        auto values = util::transformHeteroTuple<ShrinkableGet>(std::forward<ValueTuple>(valueTup));
        try {
            result =
                util::invokeWithArgTupleWithReplace<N>(func, std::forward<decltype(values)>(values), replace.get());
            // std::cout << "    test done: result=" << (result ? "true" : "false") << std::endl;
        } catch (const AssertFailed& e) {
            std::cerr << "    assertion failed: " << e.what() << std::endl;
            // std::cout << "    test failed with AssertFailed: result=" << (result ? "true" : "false") << std::endl;
            // TODO: trace
        } catch (const std::exception& e) {
            // std::cout << "    test failed with std::exception: result=" << (result ? "true" : "false") << std::endl;
            // TODO: trace
        }
        return result;
    }

    template <typename Shrinks>
    static void printShrinks(const Shrinks& shrinks)
    {
        auto itr = shrinks.iterator();
        // std::cout << "    shrinks: " << std::endl;
        for (int i = 0; i < 4 && itr.hasNext(); i++) {
            std::cout << "    ";
            show(std::cout, itr.next());
            std::cout << std::endl;
        }
    }

    template <size_t N>
    decltype(auto) shrinkN(ValueTuple&& valueTup, ShrinksTuple&& shrinksTuple)
    {
        // std::cout << "  shrinking arg " << N << ":";
        // show(std::cout, valueTup);
        // std::cout << std::endl;
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
                    // std::cout << "  shrinking arg " << N << " tested false: ";
                    // show(std::cout, valueTup);
                    // show(std::cout, next);
                    // std::cout << std::endl;
                    break;
                }
            }
            if (shrinkFound) {
                std::cout << "  shrinking found simpler failing arg " << N << ": ";
                show(std::cout, valueTup);
                std::cout << std::endl;
                if (context.hasFailures())
                    std::cout << "    by failed expectation: " << context.flushFailures(4).str() << std::endl;
            } else {
                break;
            }
        }
        // std::cout << "  no more shrinking found for arg " << N << std::endl;
        return std::get<N>(valueTup);
    }

    template <std::size_t... index>
    decltype(auto) shrinkEach(ValueTuple&& valueTup, ShrinksTuple&& shrinksTup, std::index_sequence<index...>)
    {
        return std::make_tuple(
            shrinkN<index>(std::forward<ValueTuple>(valueTup), std::forward<ShrinksTuple>(shrinksTup))...);
    }

    void shrink(Random& savedRand /*, ValueTuple&& valueTup*/)
    {
        // std::cout << "shrinking value: ";
        // show(std::cout, valueTup);
        // std::cout << std::endl;

        auto generatedValueTup = util::transformHeteroTupleWithArg<Generate>(std::forward<GenTuple>(genTup), savedRand);

        std::cout << "  with args: ";
        show(std::cout, generatedValueTup);
        std::cout << std::endl;
        // std::cout << (valueTup == valueTup2 ? "gen equals original" : "gen not equals original") << std::endl;
        static constexpr auto Size = std::tuple_size<GenTuple>::value;
        auto shrinksTuple =
            util::transformHeteroTuple<GetShrinks>(std::forward<decltype(generatedValueTup)>(generatedValueTup));
        auto shrunk = shrinkEach(std::forward<decltype(generatedValueTup)>(generatedValueTup),
                                 std::forward<decltype(shrinksTuple)>(shrinksTuple), std::make_index_sequence<Size>{});
        std::cout << "  found shrunk args: ";
        show(std::cout, shrunk);
        std::cout << std::endl;
    }

private:
    Func func;
    GenTuple genTup;
};

namespace util {

template <typename RetType, typename Callable,
          typename std::enable_if_t<std::is_same<RetType, bool>::value, bool> = true, typename... ARGS>
std::function<bool(ARGS...)> functionWithBoolResultHelper(TypeList<ARGS...>, Callable&& callable)
{
    return static_cast<std::function<RetType(ARGS...)>>(callable);
}

template <typename RetType, typename Callable,
          typename std::enable_if_t<std::is_same<RetType, void>::value, bool> = true, typename... ARGS>
std::function<bool(ARGS...)> functionWithBoolResultHelper(TypeList<ARGS...>, Callable&& callable)
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
std::function<RetType(ARGS...)> asFunctionHelper(TypeList<ARGS...>, Callable&& callable)
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
                              std::tuple<std::function<Shrinkable<std::decay_t<ARGS>>(Random&)>...>&& genTup)
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

}  // namespace PropertyBasedTesting
