#include "testing/gen.hpp"
#include "testing/generator/numeric.hpp"
#include <string>


namespace PropertyBasedTesting {


Shrinkable<int8_t> Arbitrary<int8_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return Shrinkable<int8_t>(boundaryValues[i]);
    }
    return Shrinkable<int8_t>(rand.getRandomInt8());
}

int16_t Arbitrary<int16_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return boundaryValues[i];
    }
    return rand.getRandomInt16();
}

int32_t Arbitrary<int32_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return boundaryValues[i];
    }
    return rand.getRandomInt32();
}

int64_t Arbitrary<int64_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return boundaryValues[i];
    }
    return rand.getRandomInt64();
}

uint8_t Arbitrary<uint8_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return boundaryValues[i];
    }
    return rand.getRandomUInt8();
}

uint16_t Arbitrary<uint16_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return boundaryValues[i];
    }
    return rand.getRandomUInt16();
}

uint32_t Arbitrary<uint32_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return boundaryValues[i];
    }
    return rand.getRandomUInt32();
}

uint64_t Arbitrary<uint64_t>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return boundaryValues[i];
    }
    return rand.getRandomUInt64();
}

float Arbitrary<float>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return boundaryValues[i];
    }
    return rand.getRandomFloat();
}

double Arbitrary<double>::generate(Random& rand) {
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        return boundaryValues[i];
    }
    return rand.getRandomDouble();
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
