#include "testing/gen.hpp"
#include "testing/generator/numeric.hpp"
#include "testing/generator/util.hpp"
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
    int16_t value = 0;
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        value = boundaryValues[i];
    }
    else
        value = rand.getRandomInt16();

    return binarySearchShrinkable<int16_t>(value);
}

Shrinkable<int32_t> Arbitrary<int32_t>::operator()(Random& rand) {
    int32_t value = 0;
    if(rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(boundaryValues) / sizeof(boundaryValues[0]));
        value = boundaryValues[i];
    }
    else
       value = rand.getRandomInt32();

    return binarySearchShrinkable<int32_t>(value);
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
