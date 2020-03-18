#include "testing/gen.hpp"
#include "testing/generator/numeric.hpp"
#include "testing/generator/util.hpp"
#include <string>
#include <functional>


namespace PropertyBasedTesting {


Shrinkable<int8_t> Arbitrary<int8_t>::operator()(Random& rand) {
    return generateNumeric<int8_t>(rand);
}

Shrinkable<int16_t> Arbitrary<int16_t>::operator()(Random& rand) {
    return generateNumeric<int16_t>(rand);
}

Shrinkable<int32_t> Arbitrary<int32_t>::operator()(Random& rand) {
    return generateNumeric<int32_t>(rand);
}

Shrinkable<int64_t> Arbitrary<int64_t>::operator()(Random& rand) {
    return generateNumeric<int64_t>(rand);
}

Shrinkable<uint8_t> Arbitrary<uint8_t>::operator()(Random& rand) {
    return generateNumeric<uint8_t>(rand);
}

Shrinkable<uint16_t> Arbitrary<uint16_t>::operator()(Random& rand) {
    return generateNumeric<uint16_t>(rand);
}

Shrinkable<uint32_t> Arbitrary<uint32_t>::operator()(Random& rand) {
    return generateNumeric<uint32_t>(rand);
}

Shrinkable<uint64_t> Arbitrary<uint64_t>::operator()(Random& rand) {
    return generateNumeric<uint64_t>(rand);
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
