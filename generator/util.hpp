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

template <typename T>
Shrinkable<T> binarySearchShrinkable(T value)
{
    using shrinkable_t = Shrinkable<T>;
    using stream_t = Stream<shrinkable_t>;
    using genfunc_t = typename std::function<stream_t(T, T)>;

    // given min, max, generate stream
    static genfunc_t genpos = [](T min, T max) {
        T mid = static_cast<T>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? 1 : 0));

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t(make_shrinkable<T>(mid));
        } else
            return stream_t(make_shrinkable<T>(mid).with([=]() { return genpos(min, mid); }),
                            [=]() { return genpos(mid, max); });
    };

    static genfunc_t genneg = [](T min, T max) {
        T mid = static_cast<T>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? -1 : 0));

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t(make_shrinkable<T>(mid));
        } else
            return stream_t(make_shrinkable<T>(mid).with([=]() { return genneg(mid, max); }),
                            [=]() { return genneg(min, mid); });
    };

    return make_shrinkable<T>(value).with([value]() {
        if (value == 0)
            return stream_t::empty();
        else if (value > 0)
            return stream_t(make_shrinkable<T>(0)).concat(genpos(0, value));
        else
            return stream_t(make_shrinkable<T>(0)).concat(genneg(value, 0));
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
