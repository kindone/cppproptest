#include "testing/gen.hpp"
#include "testing/generator/numeric.hpp"
#include <string>
#include <functional>


namespace PropertyBasedTesting {


Shrinkable<int8_t> Arbitrary<int8_t>::operator()(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return make_shrinkable<int8_t>(boundaryValues[i]);
    }
    return make_shrinkable<int8_t>(rand.getRandomInt8());
}
/*
Stream<Shrinkable<int8_t>> Arbitrary<int8_t>::shrinks(Shrinkable<int8_t>& target) {
    int8_t value = target;
    if(value == 0)
        return Stream.empty();
    if(value > 0)
        return Stream.fromValues(value - 1, value/2, value >> 1);
    
    return Stream.fromValues(value + 1, value/2, value >> 1);

}
*/

Shrinkable<int16_t> Arbitrary<int16_t>::operator()(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return make_shrinkable<int16_t>(boundaryValues[i]);
    }
    return make_shrinkable<int16_t>(rand.getRandomInt16());
}

Shrinkable<int32_t> Arbitrary<int32_t>::operator()(Random& rand) {
    int32_t value = 0;
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        value = boundaryValues[i];
    }
    else
       value = rand.getRandomInt32();

    using shrinkable_t = Shrinkable<int>;
    using stream_t = Stream<shrinkable_t>;
    using func_t = typename std::function<stream_t()>;
    using genfunc_t = typename std::function<stream_t(int, int)>;

    // given min, val, generate stream
    static genfunc_t genpos = [](int min, int val) {
        int mid = val/2 + min/2;
        if(val <= 0 || (val-min) <= 1 || mid == val || mid == min)
            return stream_t::empty();
        else
            return stream_t(
                make_shrinkable<int>(mid).with([=]() { return genpos(0, mid);}),
                [=]() { return genpos(mid, val); }
            );
    };

    static genfunc_t genneg = [](int max, int val) {
        int mid = val/2 + max/2;
        //std::cout << "      val: " << val << ", mid: " << mid << ", max: " << max << std::endl; 
        if(val >= 0 || (max-val) <= 1 || mid == val || mid == max)
            return stream_t::empty();
        else
            return stream_t(
                make_shrinkable<int>(mid).with([=]() { return genneg(0, mid);}),
                [=]() { return genneg(mid, val); }
            );
    };

    //std::cout << "      val0: " << value << std::endl; 
    return make_shrinkable<int>(value).with([value]() {
        //std::cout << "      val1: " << value << std::endl; 
        return  stream_t(make_shrinkable<int>(0), [value]() {
            //std::cout << "      val2: " << value << std::endl; 
            if(value >= 0)
                return genpos(0, value);
            else
                return genneg(0, value);
        });
    });
}

Shrinkable<int64_t> Arbitrary<int64_t>::operator()(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return make_shrinkable<int64_t>(boundaryValues[i]);
    }
    return make_shrinkable<int64_t>(rand.getRandomInt64());
}

Shrinkable<uint8_t> Arbitrary<uint8_t>::operator()(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return make_shrinkable<uint8_t>(boundaryValues[i]);
    }
    return make_shrinkable<uint8_t>(rand.getRandomUInt8());
}

Shrinkable<uint16_t> Arbitrary<uint16_t>::operator()(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return make_shrinkable<uint16_t>(boundaryValues[i]);
    }
    return make_shrinkable<uint16_t>(rand.getRandomUInt16());
}

Shrinkable<uint32_t> Arbitrary<uint32_t>::operator()(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return make_shrinkable<uint32_t>(boundaryValues[i]);
    }
    return make_shrinkable<uint32_t>(rand.getRandomUInt32());
}

Shrinkable<uint64_t> Arbitrary<uint64_t>::operator()(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return make_shrinkable<uint64_t>(boundaryValues[i]);
    }
    return make_shrinkable<uint64_t>(rand.getRandomUInt64());
}

Shrinkable<float> Arbitrary<float>::operator()(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return make_shrinkable<float>(boundaryValues[i]);
    }
    return make_shrinkable<float>(rand.getRandomFloat());
}

Shrinkable<double> Arbitrary<double>::operator()(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return make_shrinkable<double>(boundaryValues[i]);
    }
    return make_shrinkable<double>(rand.getRandomDouble());
}

constexpr int8_t Arbitrary<int8_t>::boundaryValues[];
constexpr int16_t Arbitrary<int16_t>::boundaryValues[];
constexpr int32_t Arbitrary<int32_t>::boundaryValues[];
constexpr int64_t Arbitrary<int64_t>::boundaryValues[];
constexpr uint8_t Arbitrary<uint8_t>::boundaryValues[];
constexpr uint16_t Arbitrary<uint16_t>::boundaryValues[];
constexpr uint32_t Arbitrary<uint32_t>::boundaryValues[];
constexpr uint64_t Arbitrary<uint64_t>::boundaryValues[];
constexpr float Arbitrary<float>::boundaryValues[];
constexpr double Arbitrary<double>::boundaryValues[];

/*
template struct Arbitrary<int8_t>;
template struct Arbitrary<int16_t>;
template struct Arbitrary<int32_t>;
template struct Arbitrary<int64_t>;
template struct Arbitrary<uint8_t>;
template struct Arbitrary<uint16_t>;
template struct Arbitrary<uint32_t>;
template struct Arbitrary<uint64_t>;
template struct Arbitrary<float>;
template struct Arbitrary<double>;
*/
} // namespace PropertyBasedTesting
