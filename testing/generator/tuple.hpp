#pragma once

#include "testing/gen.hpp"
#include "testing/Random.hpp"
#include "testing/Shrinkable.hpp"
#include "testing/tuple.hpp"
#include <tuple>


namespace PropertyBasedTesting
{

template <size_t N, typename...Shrinkables>
decltype(auto) mine(std::tuple<Shrinkables...>&& shrinkableTuple);
template <size_t N, typename...Shrinkables>
decltype(auto) rest(std::tuple<Shrinkables...>&& shrinkableTuple);

template <typename T>
struct GetValueFromShrinkable {
    static decltype(auto) transform(T&& shr) {
        return shr.get();
    }
};


template <size_t N, typename...Shrinkables>
decltype(auto) mine(std::tuple<Shrinkables...>&& shrinkableTuple) {
    using InTuple = std::tuple<Shrinkables...>;
    using OutTuple = typename std::tuple<typename Shrinkables::type...>;
    using Out = Shrinkable<OutTuple>;
    using T = typename std::tuple_element<N, OutTuple>::type; // Shrinkable<T>::type == T

    // Shrinkable(8).with(...)
    auto shrinkable = std::get<N>(shrinkableTuple);
    // (0,4,6,7)...
    auto shrinks = shrinkable.shrinks();

    // shrinkable to (0, 8, 8), (4, 8, 8), (6, 8, 8), (7, 8, 8)
    return shrinkable.template transform<OutTuple>([shrinkableTuple](const T& value) -> OutTuple {
        OutTuple valueTup = transformHeteroTuple<GetValueFromShrinkable>(std::move(shrinkableTuple));
        std::get<N>(valueTup) = value;
        return valueTup;
    });
}


template<size_t N, size_t Size, typename Tuple, std::enable_if_t<N < Size-1, bool> = true >
decltype(auto) ConcatHelper(Tuple&& shrinkableTuple) {
    return mine<N>(std::move(shrinkableTuple))/*.concat([shrinkableTuple]() {
        return rest<N+1>(std::move(const_cast<Tuple&>(shrinkableTuple)));
    })*/;
}

template<size_t N, size_t Size, typename Tuple, std::enable_if_t<N >= Size-1, bool> = false >
decltype(auto) ConcatHelper(Tuple&& shrinkableTuple) {
    return mine<N>(std::move(shrinkableTuple));
}

template <size_t N, typename...Shrinkables>
decltype(auto) rest(std::tuple<Shrinkables...>&& shrinkableTuple) {
    using InTuple = std::tuple<Shrinkables...>;
    using OutTuple = typename std::tuple<typename Shrinkables::type...>;
    using Out = Shrinkable<OutTuple>;
    using T = typename std::tuple_element<N, InTuple>::type;
    static constexpr auto Size = sizeof...(Shrinkables);

    return ConcatHelper<N, Size>(std::move(shrinkableTuple));
}

// generates (int, int)
// and shrinks one parameter by one
template <typename ... GENS>
decltype(auto) tuple(GENS&&...gens) {
    constexpr auto Size = sizeof...(GENS);

    // generator
    return [&gens...](Random& rand) {
        auto shrinkableTuple = std::make_tuple(gens(rand)...);
        auto valueTuple = std::make_tuple(gens(rand).get()...);
        using ValueTuple = decltype(valueTuple);
        return rest<0>(std::move(shrinkableTuple));

        // return make_shrinkable<ValueTuple>(valueTuple);//.with(shrinks);
    };
}


}
