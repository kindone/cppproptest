#pragma once

#include "testing/gen.hpp"
#include "testing/Random.hpp"
#include "testing/Shrinkable.hpp"
#include "testing/tuple.hpp"
#include <tuple>


namespace PropertyBasedTesting
{

template <typename ...ARGS>
class TupleGenUtility {
    using out_tuple_t = std::tuple<ARGS...>;
    using tuple_t = std::tuple<Shrinkable<ARGS>...>;
    using shrinkable_t = Shrinkable<tuple_t>;
    using stream_t = Stream<shrinkable_t>;

    static constexpr auto Size = sizeof...(ARGS);
private:


    template<size_t N, std::enable_if_t<N < sizeof...(ARGS), bool> = true >
    static shrinkable_t ConcatHelper(const shrinkable_t& aggr) {
        return ConcatHelper<N+1>(aggr.concat(genStream<N>()));
    }

    template<size_t N, std::enable_if_t<N >= sizeof...(ARGS), bool> = false >
    static shrinkable_t ConcatHelper(const shrinkable_t& aggr) {
        return aggr;
    }

    template <size_t N>
    static std::function<stream_t(const shrinkable_t&)> genStream() {
        using e_shrinkable_t = typename std::tuple_element<N, tuple_t>::type;
        using element_t = typename e_shrinkable_t::type;

        return [](const shrinkable_t& parent) -> stream_t {
            tuple_t parentRef = parent.getRef();
            if(Size == 0 || N > Size - 1)
                return stream_t::empty();

            e_shrinkable_t& elem = std::get<N>(parentRef);
            // {0,2,3} to {[x,x,x,0], ...,[x,x,x,3]}
            // make sure {1} shrinked from 2 is also transformed to [x,x,x,1]
            shrinkable_t tupWithElems = elem.template transform<tuple_t>([parentRef](const element_t& val) {
                auto copy = parentRef;
                std::get<N>(copy) = make_shrinkable<element_t>(val);
                return copy;
            });
            return tupWithElems.shrinks();
        };
    };

public:
    template <typename T>
    struct GetValueFromShrinkable {
        static decltype(auto) transform(T&& shr) {
            return shr.get();
        }
    };

    static Shrinkable<out_tuple_t> generateStream(const shrinkable_t& shrinkable) {
        return ConcatHelper<0>(shrinkable).template transform<out_tuple_t>([](const tuple_t& tuple){
            return transformHeteroTuple<GetValueFromShrinkable>(std::move(tuple));
        });
    }
};


template <typename...ARGS>
Shrinkable<std::tuple<ARGS...>> generateTupleStream(const Shrinkable<std::tuple<Shrinkable<ARGS>...>>& shrinkable) {
    return TupleGenUtility<ARGS...>::generateStream(shrinkable);
}

// generates e.g. (int, int)
// and shrinks one parameter by one and then continues to the next
template <typename ... GENS, std::enable_if_t<0 < sizeof...(GENS), bool> = true>
decltype(auto) tuple(GENS&&...gens) {
    constexpr auto Size = sizeof...(GENS);

    // generator
    return [&gens...](Random& rand) {
        auto elemTup = std::make_tuple(gens(rand)...);
        auto shrinkable = make_shrinkable<decltype(elemTup)>(elemTup);
        return generateTupleStream(shrinkable);
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
