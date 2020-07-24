#pragma once

#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include <utility>
#include <memory>

namespace proptest {

namespace util {

template <typename ARG1, typename ARG2>
class PairGenUtility {
    using out_pair_t = std::pair<ARG1, ARG2>;
    using pair_t = std::pair<Shrinkable<ARG1>, Shrinkable<ARG2>>;
    using shrinkable_t = Shrinkable<pair_t>;
    using stream_t = Stream<shrinkable_t>;

private:
    static std::function<stream_t(const shrinkable_t&)> genStream1()
    {
        using e_shrinkable_t = Shrinkable<ARG1>;
        using element_t = typename e_shrinkable_t::type;

        static auto gen = [](const shrinkable_t& parent) -> stream_t {
            std::shared_ptr<pair_t> parentRef = parent.getSharedPtr();

            e_shrinkable_t& elem = parentRef->first;
            shrinkable_t pairWithElems = elem.template transform<pair_t>([parentRef](const element_t& val) {
                auto copy = *parentRef;
                copy.first = make_shrinkable<element_t>(val);
                return make_shrinkable<pair_t>(copy);
            });
            return pairWithElems.shrinks();
        };
        return gen;
    }

    static std::function<stream_t(const shrinkable_t&)> genStream2()
    {
        using e_shrinkable_t = Shrinkable<ARG2>;
        using element_t = typename e_shrinkable_t::type;

        static auto gen = [](const shrinkable_t& parent) -> stream_t {
            std::shared_ptr<pair_t> parentRef = parent.getSharedPtr();

            e_shrinkable_t& elem = parentRef->second;
            shrinkable_t pairWithElems = elem.template transform<pair_t>([parentRef](const element_t& val) {
                auto copy = *parentRef;
                copy.second = make_shrinkable<element_t>(val);
                return make_shrinkable<pair_t>(copy);
            });
            return pairWithElems.shrinks();
        };
        return gen;
    }

public:
    static Shrinkable<out_pair_t> generateStream(const shrinkable_t& shrinkable)
    {
        auto concatenated = shrinkable.concat(genStream1()).concat(genStream2());
        return concatenated.template transform<out_pair_t>([](const pair_t& pair) {
            return make_shrinkable<out_pair_t>(std::make_pair(pair.first.get(), pair.second.get()));
        });
    }
};

template <typename ARG1, typename ARG2>
Shrinkable<std::pair<ARG1, ARG2>> generatePairStream(
    const Shrinkable<std::pair<Shrinkable<ARG1>, Shrinkable<ARG2>>>& shrinkable)
{
    return PairGenUtility<ARG1, ARG2>::generateStream(shrinkable);
}

}  // namespace util

// generates e.g. (int, int)
// and shrinks one parameter by one and then continues to the next
template <typename GEN1, typename GEN2>
decltype(auto) pair(GEN1&& gen1, GEN2&& gen2)
{
    auto genPairPtr = std::make_shared<std::pair<std::decay_t<GEN1>, std::decay_t<GEN2>>>(std::forward<GEN1>(gen1),
                                                                                          std::forward<GEN2>(gen2));
    // generator
    return customGen([genPairPtr](Random& rand) mutable {
        auto elemPair = std::make_pair(genPairPtr->first(rand), genPairPtr->second(rand));
        auto shrinkable = make_shrinkable<decltype(elemPair)>(elemPair);
        return util::generatePairStream(shrinkable);
    });
}

template <typename ARG1, typename ARG2>
class PROPTEST_API Arbitrary<std::pair<ARG1, ARG2>> final : public ArbitraryBase<std::pair<ARG1, ARG2>> {
public:
    Shrinkable<std::pair<ARG1, ARG2>> operator()(Random& rand) override
    {
        return pair(Arbitrary<ARG1>(), Arbitrary<ARG2>())(rand);
    }
};

}  // namespace proptest
