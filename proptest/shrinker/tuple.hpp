#pragma once

#include "../Shrinkable.hpp"
#include "../util/tuple.hpp"

namespace proptest {


namespace util {

template <typename... ARGS>
class TupleShrinker {
    using out_tuple_t = tuple<ARGS...>;
    using tuple_t = tuple<Shrinkable<ARGS>...>;
    using shrinkable_t = Shrinkable<tuple_t>;
    using stream_t = Stream;

    static constexpr auto Size = sizeof...(ARGS);

private:
    template <size_t N>
        requires (N < sizeof...(ARGS))
    static shrinkable_t ConcatHelper(const shrinkable_t& aggr)
    {
        return ConcatHelper<N + 1>(aggr.concat(genStream<N>()));
    }

    template <size_t N>
        requires (N >= sizeof...(ARGS))
    static shrinkable_t ConcatHelper(const shrinkable_t& aggr)
    {
        return aggr;
    }

    template <size_t N>
    static function<stream_t(const shrinkable_t&)> genStream()
    {
        using e_shrinkable_t = typename tuple_element<N, tuple_t>::type;
        using element_t = typename e_shrinkable_t::type;

        return +[](const shrinkable_t& parent) -> stream_t {
            if (Size == 0 || N > Size - 1)
                return stream_t::empty();

            shared_ptr<tuple_t> parentRef = util::make_shared<tuple_t>(parent.getRef());

            e_shrinkable_t& elem = get<N>(*parentRef);
            // {0,2,3} to {[x,x,x,0], ...,[x,x,x,3]}
            // make sure {1} shrinked from 2 is also transformed to [x,x,x,1]
            shrinkable_t tupWithElems = elem.template flatMap<tuple_t>([parentRef](const element_t& val) {
                get<N>(*parentRef) = make_shrinkable<element_t>(val);
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

    static Shrinkable<out_tuple_t> shrink(const shrinkable_t& shrinkable)
    {
        return ConcatHelper<0>(shrinkable).template flatMap<out_tuple_t>(+[](const tuple_t& tuple) {
            return make_shrinkable<out_tuple_t>(transformHeteroTuple<GetValueFromShrinkable>(util::move(tuple)));
        });
    }
};

} // namespace util

template <typename... ARGS>
Shrinkable<tuple<ARGS...>> shrinkTuple(const Shrinkable<tuple<Shrinkable<ARGS>...>>& shrinkable)
{
    return util::TupleShrinker<ARGS...>::shrink(shrinkable);
}

} // namespace proptest
