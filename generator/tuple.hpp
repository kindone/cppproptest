#pragma once

#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "../util/tuple.hpp"
#include <tuple>
#include <memory>

namespace proptest {

namespace util {

template <typename... ARGS>
class TupleGenUtility {
    using out_tuple_t = std::tuple<ARGS...>;
    using tuple_t = std::tuple<Shrinkable<ARGS>...>;
    using shrinkable_t = Shrinkable<tuple_t>;
    using stream_t = Stream<shrinkable_t>;

    static constexpr auto Size = sizeof...(ARGS);

private:
    template <size_t N>
        static std::enable_if_t < N<sizeof...(ARGS), shrinkable_t> ConcatHelper(const shrinkable_t& aggr)
    {
        return ConcatHelper<N + 1>(aggr.concat(genStream<N>()));
    }

    template <size_t N>
    static std::enable_if_t<N >= sizeof...(ARGS), shrinkable_t> ConcatHelper(const shrinkable_t& aggr)
    {
        return aggr;
    }

    template <size_t N>
    static std::function<stream_t(const shrinkable_t&)> genStream()
    {
        using e_shrinkable_t = typename std::tuple_element<N, tuple_t>::type;
        using element_t = typename e_shrinkable_t::type;

        return +[](const shrinkable_t& parent) -> stream_t {
            if (Size == 0 || N > Size - 1)
                return stream_t::empty();

            std::shared_ptr<tuple_t> parentRef = std::make_shared<tuple_t>(parent.getRef());

            e_shrinkable_t& elem = std::get<N>(*parentRef);
            // {0,2,3} to {[x,x,x,0], ...,[x,x,x,3]}
            // make sure {1} shrinked from 2 is also transformed to [x,x,x,1]
            shrinkable_t tupWithElems = elem.template transform<tuple_t>([parentRef](const element_t& val) {
                std::get<N>(*parentRef) = make_shrinkable<element_t>(val);
                return make_shrinkable<tuple_t>(*parentRef);
            });
            return tupWithElems.shrinks();
        };
    }

public:
    template <typename T>
    struct GetValueFromShrinkable
    {
        static decltype(auto) transform(T&& shr) { return shr.get(); }
    };

    static Shrinkable<out_tuple_t> generateStream(const shrinkable_t& shrinkable)
    {
        return ConcatHelper<0>(shrinkable).template transform<out_tuple_t>(+[](const tuple_t& tuple) {
            return make_shrinkable<out_tuple_t>(transformHeteroTuple<GetValueFromShrinkable>(std::move(tuple)));
        });
    }
};

template <typename... ARGS>
Shrinkable<std::tuple<ARGS...>> generateTupleStream(const Shrinkable<std::tuple<Shrinkable<ARGS>...>>& shrinkable)
{
    return TupleGenUtility<ARGS...>::generateStream(shrinkable);
}

}  // namespace util

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
        return util::generateTupleStream(shrinkable);
    });
}

template <typename... ARGS>
class PROPTEST_API Arbitrary<std::tuple<ARGS...>> final : public ArbitraryBase<std::tuple<ARGS...>> {
public:
    Shrinkable<std::tuple<ARGS...>> operator()(Random& rand) override { return tuple(Arbitrary<ARGS>()...)(rand); }
};

}  // namespace proptest
