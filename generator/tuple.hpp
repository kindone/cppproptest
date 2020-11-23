#pragma once

#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "../util/tuple.hpp"
#include "../shrinker/tuple.hpp"
#include "util.hpp"
#include <tuple>
#include <memory>

namespace proptest {

// generates e.g. (int, int)
// and shrinks one parameter by one and then continues to the next
template <typename GEN0, typename... GENS>
decltype(auto) tuple(GEN0&& gen0, GENS&&... gens)
{
    // constexpr auto Size = sizeof...(GENS);
    std::tuple<std::decay_t<GEN0>, std::decay_t<GENS>...> genTup = std::make_tuple(gen0, gens...);
    // generator
    return generator([genTup](Random& rand) mutable {
        auto elemTup = util::transformHeteroTupleWithArg<util::Generate>(std::forward<decltype(genTup)>(genTup), rand);
        auto shrinkable = make_shrinkable<decltype(elemTup)>(elemTup);
        return shrinkTuple(shrinkable);
    });
}

template <typename... ARGS>
class PROPTEST_API Arbi<std::tuple<ARGS...>> final : public ArbiBase<std::tuple<ARGS...>> {
public:
    Shrinkable<std::tuple<ARGS...>> operator()(Random& rand) override { return tuple(Arbi<ARGS>()...)(rand); }
};

}  // namespace proptest
