#pragma once
#include "../Shrinkable.hpp"

namespace proptest {

namespace util {

template <typename ARG1, typename ARG2>
class PairShrinker {
    using out_pair_t = std::pair<ARG1, ARG2>;
    using pair_t = std::pair<Shrinkable<ARG1>, Shrinkable<ARG2>>;
    using shrinkable_t = Shrinkable<pair_t>;
    using stream_t = Stream<shrinkable_t>;

private:
    static std::function<stream_t(const shrinkable_t&)> shrinkFirst()
    {
        using e_shrinkable_t = Shrinkable<ARG1>;
        using element_t = typename e_shrinkable_t::type;

        static auto gen = +[](const shrinkable_t& parent) -> stream_t {
            std::shared_ptr<pair_t> parentRef = parent.getSharedPtr();

            e_shrinkable_t& elem = parentRef->first;
            shrinkable_t pairWithElems = elem.template flatMap<pair_t>([parentRef](const element_t& val) {
                auto copy = *parentRef;
                copy.first = make_shrinkable<element_t>(val);
                return make_shrinkable<pair_t>(copy);
            });
            return pairWithElems.shrinks();
        };
        return gen;
    }

    static std::function<stream_t(const shrinkable_t&)> shrinkSecond()
    {
        using e_shrinkable_t = Shrinkable<ARG2>;
        using element_t = typename e_shrinkable_t::type;

        static auto gen = +[](const shrinkable_t& parent) -> stream_t {
            std::shared_ptr<pair_t> parentRef = parent.getSharedPtr();

            e_shrinkable_t& elem = parentRef->second;
            shrinkable_t pairWithElems = elem.template flatMap<pair_t>([parentRef](const element_t& val) {
                auto copy = *parentRef;
                copy.second = make_shrinkable<element_t>(val);
                return make_shrinkable<pair_t>(copy);
            });
            return pairWithElems.shrinks();
        };
        return gen;
    }

public:
    static Shrinkable<out_pair_t> shrink(const shrinkable_t& shrinkable)
    {
        auto concatenated = shrinkable.concat(shrinkFirst()).concat(shrinkSecond());
        return concatenated.template flatMap<out_pair_t>(+[](const pair_t& pair) {
            return make_shrinkable<out_pair_t>(std::make_pair(pair.first.get(), pair.second.get()));
        });
    }
};

} // namespace util

template <typename ARG1, typename ARG2>
Shrinkable<std::pair<ARG1, ARG2>> shrinkPair(
    const Shrinkable<ARG1>& firstShr, const Shrinkable<ARG2>& secondShr)
{
    auto elemPair = std::make_pair(firstShr, secondShr);
    auto shrinkable = make_shrinkable<decltype(elemPair)>(elemPair);
    return util::PairShrinker<ARG1, ARG2>::shrink(shrinkable);
}

} // namespace proptest
