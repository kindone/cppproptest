#include "testing/gen.hpp"
#include "testing/generator/numeric.hpp"
#include <string>
#include <functional>


namespace PropertyBasedTesting {


Shrinkable<int8_t> Arbitrary<int8_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return Shrinkable<int8_t>(boundaryValues[i]);
    }
    return Shrinkable<int8_t>(rand.getRandomInt8());
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

Shrinkable<int16_t> Arbitrary<int16_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return Shrinkable<int16_t>(boundaryValues[i]);
    }
    return Shrinkable<int16_t>(rand.getRandomInt16());
}

Shrinkable<int32_t> Arbitrary<int32_t>::generate(Random& rand) {
    uint32_t value = 0;
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        value = boundaryValues[i];
    }
    else
       value = rand.getRandomInt32();

    return Shrinkable<int>(value, [value]() {
        return  Stream<Shrinkable<int>>(0, [value]() {
            static std::function<Stream<Shrinkable<int>>(int,int)> genpos = [](int min, int val) {
                if(val <= 0 || (val-min) <= 1)
                    return Stream<Shrinkable<int>>::empty();
                else
                    return Stream<Shrinkable<int>>(Shrinkable<int>((val + min)/2), [min, val]() { return genpos((val+min)/2, val); });
            };
            static std::function<Stream<Shrinkable<int>>(int,int)> genneg = [](int max, int val) {
                if(val >= 0 || (max-val) <= 1)
                    return Stream<Shrinkable<int>>::empty();
                else
                    return Stream<Shrinkable<int>>(Shrinkable<int>((val + max)/2), [max, val]() { return genneg((val+max)/2, val); });
            };
            if(value >= 0)
                return genpos(0, value);
            else
                return genneg(0, value);
        });
    });
}

Shrinkable<int64_t> Arbitrary<int64_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return Shrinkable<int64_t>(boundaryValues[i]);
    }
    return Shrinkable<int64_t>(rand.getRandomInt64());
}

Shrinkable<uint8_t> Arbitrary<uint8_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return Shrinkable<uint8_t>(boundaryValues[i]);
    }
    return Shrinkable<uint8_t>(rand.getRandomUInt8());
}

Shrinkable<uint16_t> Arbitrary<uint16_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return Shrinkable<uint16_t>(boundaryValues[i]);
    }
    return Shrinkable<uint16_t>(rand.getRandomUInt16());
}

Shrinkable<uint32_t> Arbitrary<uint32_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return Shrinkable<uint32_t>(boundaryValues[i]);
    }
    return Shrinkable<uint32_t>(rand.getRandomUInt32());
}

Shrinkable<uint64_t> Arbitrary<uint64_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return Shrinkable<uint64_t>(boundaryValues[i]);
    }
    return Shrinkable<uint64_t>(rand.getRandomUInt64());
}

Shrinkable<float> Arbitrary<float>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return Shrinkable<float>(boundaryValues[i]);
    }
    return Shrinkable<float>(rand.getRandomFloat());
}

Shrinkable<double> Arbitrary<double>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return Shrinkable<double>(boundaryValues[i]);
    }
    return Shrinkable<double>(rand.getRandomDouble());
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
