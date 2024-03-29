#include "util.hpp"
#include "../Random.hpp"
#include "../util/std.hpp"

namespace proptest {
namespace util {

Shrinkable<int64_t> binarySearchShrinkable(int64_t value)
{
    using stream_t = Stream;
    using genfunc_t = function<stream_t(int64_t, int64_t)>;

    // given min, max, generate stream
    static genfunc_t genpos = +[](int64_t min, int64_t max) {
        int64_t mid = static_cast<int64_t>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? 1 : 0));

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t(make_shrinkable_any<int64_t>(mid));
        } else
            return stream_t(make_shrinkable_any<int64_t>(mid).with([=]() { return genpos(min, mid); }),
                            [=]() { return genpos(mid, max); });
    };

    static genfunc_t genneg = +[](int64_t min, int64_t max) {
        int64_t mid = static_cast<int64_t>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? -1 : 0));

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t(make_shrinkable_any<int64_t>(mid));
        } else
            return stream_t(make_shrinkable_any<int64_t>(mid).with([=]() { return genneg(mid, max); }),
                            [=]() { return genneg(min, mid); });
    };

    return make_shrinkable<int64_t>(value).with([value]() {
        if (value == 0)
            return stream_t::empty();
        else if (value > 0)
            return stream_t(make_shrinkable_any<int64_t>(0)).concat(genpos(0, value));
        else
            return stream_t(make_shrinkable_any<int64_t>(0)).concat(genneg(value, 0));
    });
}

Shrinkable<uint64_t> binarySearchShrinkableU(uint64_t value)
{
    using stream_t = Stream;
    using genfunc_t = function<stream_t(uint64_t, uint64_t)>;

    // given min, max, generate stream
    static genfunc_t genpos = +[](uint64_t min, uint64_t max) {
        uint64_t mid = static_cast<uint64_t>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? 1 : 0));

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t(make_shrinkable_any<uint64_t>(mid));
        } else
            return stream_t(make_shrinkable_any<uint64_t>(mid).with([=]() { return genpos(min, mid); }),
                            [=]() { return genpos(mid, max); });
    };

    static genfunc_t genneg = +[](uint64_t min, uint64_t max) {
        uint64_t mid = static_cast<uint64_t>(min / 2 + max / 2 + ((min % 2 != 0 && max % 2 != 0) ? -1 : 0));

        if (min + 1 >= max) {
            return stream_t::empty();
        } else if (min + 2 >= max) {
            return stream_t(make_shrinkable_any<uint64_t>(mid));
        } else
            return stream_t(make_shrinkable_any<uint64_t>(mid).with([=]() { return genneg(mid, max); }),
                            [=]() { return genneg(min, mid); });
    };

    return make_shrinkable<uint64_t>(value).with([value]() {
        if (value == 0)
            return stream_t::empty();
        else if (value > 0)
            return stream_t(make_shrinkable_any<uint64_t>(0)).concat(genpos(0, value));
        else
            return stream_t(make_shrinkable_any<uint64_t>(0)).concat(genneg(value, 0));
    });
}

}  // namespace util
}  // namespace proptest
