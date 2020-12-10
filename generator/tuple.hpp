#pragma once

#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "../util/tuple.hpp"
#include "../util/std.hpp"
#include "../shrinker/tuple.hpp"
#include "util.hpp"


namespace proptest {

// generates e.g. (int, int)
// and shrinks one parameter by one and then continues to the next
template <typename GEN0, typename... GENS>
decltype(auto) tupleOf(GEN0&& gen0, GENS&&... gens)
{
    // constexpr auto Size = sizeof...(GENS);
    tuple<decay_t<GEN0>, decay_t<GENS>...> genTup = util::make_tuple(gen0, gens...);
    // generator
    return generator([genTup](Random& rand) mutable {
        auto elemTup = util::transformHeteroTupleWithArg<util::Generate>(util::forward<decltype(genTup)>(genTup), rand);
        auto shrinkable = make_shrinkable<decltype(elemTup)>(elemTup);
        return shrinkTuple(shrinkable);
    });
}

template <typename... ARGS>
class PROPTEST_API Arbi<tuple<ARGS...>> final : public ArbiBase<tuple<ARGS...>> {
public:
    Shrinkable<tuple<ARGS...>> operator()(Random& rand) override { return tupleOf(Arbi<ARGS>()...)(rand); }
};

}  // namespace proptest
