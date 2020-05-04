#pragma once
#include "../api.hpp"
#include "../Shrinkable.hpp"
#include "../Stream.hpp"
#include "../Random.hpp"
#include <functional>
#include <utility>
#include <tuple>

namespace PropertyBasedTesting {

class Random;

template <typename INTTYPE>
Shrinkable<INTTYPE> binarySearchShrinkable(INTTYPE value)
{
    using shrinkable_t = Shrinkable<INTTYPE>;
    using stream_t = Stream<shrinkable_t>;
    // using func_t = typename std::function<stream_t()>;
    using genfunc_t = typename std::function<stream_t(INTTYPE, INTTYPE)>;

    // given min, max, generate stream
    static genfunc_t genpos = [](INTTYPE min, INTTYPE max) {
        INTTYPE mid = static_cast<INTTYPE>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? 1 : 0));
        // std::cout << "      min: " << min << ", mid: " << mid << ", max: " << max << std::endl;
        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t(make_shrinkable<INTTYPE>(mid));
        } else
            return stream_t(make_shrinkable<INTTYPE>(mid).with([=]() { return genpos(min, mid); }),
                            [=]() { return genpos(mid, max); });
    };

    static genfunc_t genneg = [](INTTYPE min, INTTYPE max) {
        INTTYPE mid = static_cast<INTTYPE>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? -1 : 0));
        // std::cout << "      min: " << min << ", mid: " << mid << ", max: " << max << std::endl;
        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t(make_shrinkable<INTTYPE>(mid));
        } else
            return stream_t(make_shrinkable<INTTYPE>(mid).with([=]() { return genneg(mid, max); }),
                            [=]() { return genneg(min, mid); });
    };

    // std::cout << "      val0: " << value << std::endl;
    return make_shrinkable<INTTYPE>(value).with([value]() {
        if (value == 0)
            return stream_t::empty();
        else if (value > 0)
            return stream_t(make_shrinkable<INTTYPE>(0)).concat(genpos(0, value));
        else
            return stream_t(make_shrinkable<INTTYPE>(0)).concat(genneg(value, 0));
    });
}

template <typename T>
decltype(auto) GetShrinksHelper(const Shrinkable<T>& shr)
{
    return shr.shrinks();
}

template <typename T>
struct GetShrinks
{
    static Stream<T> transform(T&& v) { return GetShrinksHelper(v); }
};

template <typename T>
struct Generate
{
    static decltype(auto) transform(T&& gen, Random& rand) { return gen(rand); }
};

template <typename T>
struct ShrinkableGet
{
    static decltype(auto) transform(T&& shrinkable) { return shrinkable.get(); }
};

template <typename T>
struct ShrinkableGetRef
{
    static decltype(auto) transform(T&& shrinkable) { return shrinkable.getRef(); }
};

}  // namespace PropertyBasedTesting
