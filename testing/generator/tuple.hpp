#pragma once

#include "testing/gen.hpp"
#include "testing/Random.hpp"
#include "testing/Shrinkable.hpp"
#include "testing/tuple.hpp"
#include <tuple>


namespace PropertyBasedTesting
{

template <typename...ARGS>
std::function<Shrinkable<std::tuple<ARGS...>>()> genTuple(const std::tuple<Shrinkable<ARGS>...>& shrinkableTuple);

template <typename T>
struct GetValueFromShrinkable {
    static decltype(auto) transform(T&& shr) {
        return shr.get();
    }
};

template<size_t N, typename ...ARGS, typename Tuple, std::enable_if_t<N < sizeof...(ARGS), bool> = true >
Stream<Shrinkable<std::tuple<ARGS...>>> ConcatHelper(const Stream<Shrinkable<std::tuple<ARGS...>>>& aggr, Tuple&& tuple) {
    const Stream<Shrinkable<std::tuple<ARGS...>>>& base = aggr.concat(std::get<N>(tuple));
    return ConcatHelper<N+1>(base, tuple);
}

template<size_t N, typename ...ARGS, typename Tuple, std::enable_if_t<N >= sizeof...(ARGS), bool> = false >
Stream<Shrinkable<std::tuple<ARGS...>>> ConcatHelper(const Stream<Shrinkable<std::tuple<ARGS...>>>& aggr, Tuple&& shrinkableTuple) {
    return aggr;
}

template<size_t N, typename...ARGS>
Stream<Shrinkable<std::tuple<ARGS...>>> joinShrinkables(const std::tuple<Shrinkable<ARGS>...>& shrinkableTuple) {
    using ShrinkableTuple = std::tuple<Shrinkable<ARGS>...>;
    using ShrinkableType = typename std::tuple_element<N, ShrinkableTuple>::type;
    using OutTuple = std::tuple<ARGS...>;
    using T = typename ShrinkableType::type;

    ShrinkableType shrinkable = std::get<N>(shrinkableTuple);
    Stream<ShrinkableType> shrinks = shrinkable.shrinks();
    return shrinks.template transform<Shrinkable<OutTuple>>([shrinkableTuple](const ShrinkableType& shr) mutable {
        //FIXME: copy twice?
        ShrinkableTuple copy = shrinkableTuple; // std::tuple<Shrinkable<ARGS>...>
        // replace N's element in tuple
        std::get<N>(copy) = shr;
        return genTuple(copy)(); // Shrinkable<std::tuple<ARGS...>>
    });
}

template<typename...ARGS, std::size_t...index>
Stream<Shrinkable<std::tuple<ARGS...>>> joinShrinkablesHelper(const std::tuple<Shrinkable<ARGS>...>& shrinkableTuple, std::index_sequence<index...>) {
    auto joinedShrinkables = std::make_tuple(joinShrinkables<index>(shrinkableTuple)...);
    Stream<Shrinkable<std::tuple<ARGS...>>>& base = std::get<0>(joinedShrinkables);
    return ConcatHelper<1>(base, joinedShrinkables);
}

template <typename...ARGS>
std::function<Shrinkable<std::tuple<ARGS...>>()> genTuple(const std::tuple<Shrinkable<ARGS>...>& shrinkableTuple) {
    using ShrinkableTuple = std::tuple<Shrinkable<ARGS>...>;
    using OutTuple = std::tuple<ARGS...>;
    static constexpr auto Size = sizeof...(ARGS);

    return [&shrinkableTuple]() -> Shrinkable<OutTuple> {
        OutTuple valueTup = transformHeteroTuple<GetValueFromShrinkable>(std::move(shrinkableTuple));
        Stream<Shrinkable<OutTuple>> shrinks = joinShrinkablesHelper(shrinkableTuple, std::make_index_sequence<Size>{});
        return make_shrinkable<OutTuple>(valueTup).with([shrinks]() {
            return shrinks;
        });
    };
}


// generates (int, int)
// and shrinks one parameter by one
template <typename ... GENS>
decltype(auto) tuple(GENS&&...gens) {
    constexpr auto Size = sizeof...(GENS);

    // generator
    return [&gens...](Random& rand) {
        auto shrinkableTuple = std::make_tuple(gens(rand)...);
        return genTuple(shrinkableTuple)();
    };
}

template <typename ...ARGS>
class PROPTEST_API Arbitrary< std::tuple<ARGS...>> : public Gen< std::tuple<ARGS...> >
{
public:
    Shrinkable<std::tuple<ARGS...>> operator()(Random& rand) {
        return tuple(Arbitrary<ARGS>()...)(rand);
    }
};

}
