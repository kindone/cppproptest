#pragma once

#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "../shrinker/pair.hpp"
#include <utility>
#include <memory>

namespace proptest {


// generates e.g. (int, int)
// and shrinks one parameter by one and then continues to the next
template <typename GEN1, typename GEN2>
decltype(auto) pair(GEN1&& gen1, GEN2&& gen2)
{
    auto genPairPtr = std::make_shared<std::pair<std::decay_t<GEN1>, std::decay_t<GEN2>>>(std::forward<GEN1>(gen1),
                                                                                          std::forward<GEN2>(gen2));
    // generator
    return generator([genPairPtr](Random& rand) mutable {
        return shrinkPair(genPairPtr->first(rand), genPairPtr->second(rand));
    });
}

template <typename ARG1, typename ARG2>
class PROPTEST_API Arbi<std::pair<ARG1, ARG2>> final : public ArbiBase<std::pair<ARG1, ARG2>> {
public:
    Shrinkable<std::pair<ARG1, ARG2>> operator()(Random& rand) override
    {
        return pair(Arbi<ARG1>(), Arbi<ARG2>())(rand);
    }
};

}  // namespace proptest
