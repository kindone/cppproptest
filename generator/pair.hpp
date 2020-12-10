#pragma once

#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "../shrinker/pair.hpp"
#include "../util/std.hpp"

namespace proptest {


// generates e.g. (int, int)
// and shrinks one parameter by one and then continues to the next
template <typename GEN1, typename GEN2>
decltype(auto) pairOf(GEN1&& gen1, GEN2&& gen2)
{
    auto genPairPtr = util::make_shared<pair<decay_t<GEN1>, decay_t<GEN2>>>(util::forward<GEN1>(gen1),
                                                                                          util::forward<GEN2>(gen2));
    // generator
    return generator([genPairPtr](Random& rand) mutable {
        return shrinkPair(genPairPtr->first(rand), genPairPtr->second(rand));
    });
}

template <typename ARG1, typename ARG2>
class PROPTEST_API Arbi<pair<ARG1, ARG2>> final : public ArbiBase<pair<ARG1, ARG2>> {
public:
    Shrinkable<pair<ARG1, ARG2>> operator()(Random& rand) override
    {
        return pairOf(Arbi<ARG1>(), Arbi<ARG2>())(rand);
    }
};

}  // namespace proptest
