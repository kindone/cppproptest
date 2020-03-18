#pragma once
#include "testing/api.hpp"
#include "testing/Shrinkable.hpp"
#include "testing/Stream.hpp"

namespace PropertyBasedTesting {


template <typename INTTYPE>
decltype(auto) binarySearchShrinkable(INTTYPE value) {
    using shrinkable_t = Shrinkable<INTTYPE>;
    using stream_t = Stream<shrinkable_t>;
    using func_t = typename std::function<stream_t()>;
    using genfunc_t = typename std::function<stream_t(INTTYPE, INTTYPE)>;

    // given min, max, generate stream
    static genfunc_t genpos = [](INTTYPE min, INTTYPE max) {
        INTTYPE mid = min/2 + max/2;
        //std::cout << "      min: " << min << ", mid: " << mid << ", max: " << max << std::endl; 
        if(mid+1 >= max || min+1 >= mid)
            return stream_t(make_shrinkable<INTTYPE>(mid));
        else if(mid+1 >= max)
            return stream_t(make_shrinkable<INTTYPE>(mid).with([=]() { return genpos(min, mid);}));
        else if(min+1 >= mid)
            return stream_t(
                make_shrinkable<INTTYPE>(mid),
                [=]() { return genpos(mid, max); }
            );
        else
            return stream_t(
                make_shrinkable<INTTYPE>(mid).with([=]() { return genpos(min, mid);}),
                [=]() { return genpos(mid, max); }
            );
    };

    static genfunc_t genneg = [](INTTYPE min, INTTYPE max) {
        INTTYPE mid = min/2 + max/2;
        //std::cout << "      min: " << min << ", mid: " << mid << ", max: " << max << std::endl; 
        if(mid+1 >= max || min+1 >= mid)
            return stream_t(make_shrinkable<INTTYPE>(mid));
        else if(mid+1 >= max)
            return stream_t(make_shrinkable<INTTYPE>(mid).with([=]() { return genneg(mid, max);}));
        else if(min+1 >= mid)
            return stream_t(
                make_shrinkable<INTTYPE>(mid),
                [=]() { return genneg(min, mid); }
            );
        else
            return stream_t(
                make_shrinkable<INTTYPE>(mid).with([=]() { return genneg(mid, max);}),
                [=]() { return genneg(min, mid); }
            );
    };

    //std::cout << "      val0: " << value << std::endl; 
    return make_shrinkable<INTTYPE>(value).with([value]() {
        //std::cout << "      val1: " << value << std::endl; 
        return  stream_t(make_shrinkable<INTTYPE>(0), [value]() {
            //std::cout << "      val2: " << value << std::endl; 
            if(value >= 0)
                return genpos(0, value);
            else
                return genpos(value, 0);
        });
    });
}

} // namespace}
