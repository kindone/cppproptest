#include "../gen.hpp"
#include "numeric.hpp"
#include "bool.hpp"
#include "util.hpp"
#include "tuple.hpp"
#include <string>
#include <functional>
#include <cmath>

namespace pbt {

Shrinkable<char> Arbitrary<char>::operator()(Random& rand)
{
    return generateInteger<char>(rand);
}

Shrinkable<int8_t> Arbitrary<int8_t>::operator()(Random& rand)
{
    return generateInteger<int8_t>(rand);
}

Shrinkable<int16_t> Arbitrary<int16_t>::operator()(Random& rand)
{
    return generateInteger<int16_t>(rand);
}

Shrinkable<int32_t> Arbitrary<int32_t>::operator()(Random& rand)
{
    return generateInteger<int32_t>(rand);
}

Shrinkable<int64_t> Arbitrary<int64_t>::operator()(Random& rand)
{
    return generateInteger<int64_t>(rand);
}

Shrinkable<uint8_t> Arbitrary<uint8_t>::operator()(Random& rand)
{
    return generateInteger<uint8_t>(rand);
}

Shrinkable<uint16_t> Arbitrary<uint16_t>::operator()(Random& rand)
{
    return generateInteger<uint16_t>(rand);
}

Shrinkable<uint32_t> Arbitrary<uint32_t>::operator()(Random& rand)
{
    return generateInteger<uint32_t>(rand);
}

Shrinkable<uint64_t> Arbitrary<uint64_t>::operator()(Random& rand)
{
    return generateInteger<uint64_t>(rand);
}

namespace util {

template <typename FLOATTYPE>
FLOATTYPE decomposeFloat(FLOATTYPE value, int* exp);

template <>
float decomposeFloat<float>(float value, int* exp)
{
    return frexpf(value, exp);
}

template <>
double decomposeFloat<double>(double value, int* exp)
{
    return frexp(value, exp);
}

template <typename FLOATTYPE>
FLOATTYPE composeFloat(FLOATTYPE value, int exp);

template <>
float composeFloat<float>(float value, int exp)
{
    return ldexpf(value, exp);
}

template <>
double composeFloat<double>(double value, int exp)
{
    return ldexp(value, exp);
}

}  // namespace util

template <typename FLOATTYPE>
Stream<Shrinkable<FLOATTYPE>> shrinkFloat(FLOATTYPE value)
{
    int exp = 0;
    if (value == 0.0f) {
        return Stream<Shrinkable<FLOATTYPE>>::empty();
    } else if (std::isnan(value)) {
        return Stream<Shrinkable<FLOATTYPE>>::one(make_shrinkable<FLOATTYPE>(0.0f));
    } else {
        FLOATTYPE fraction = 0.0f;
        if (std::isinf(value)) {
            if (value > 0) {
                auto max = std::numeric_limits<FLOATTYPE>::max();
                fraction = util::decomposeFloat(max, &exp);
            } else {
                auto min = std::numeric_limits<FLOATTYPE>::lowest();
                fraction = util::decomposeFloat(min, &exp);
            }
        } else {
            fraction = util::decomposeFloat(value, &exp);
        }

        auto expShrinkable = binarySearchShrinkable(exp);
        // shrink exponent
        auto floatShrinkable = expShrinkable.transform<FLOATTYPE>(
            [fraction](const int& exp) { return util::composeFloat(fraction, exp); });

        // prepend 0.0
        floatShrinkable = floatShrinkable.with([shrinksPtr = floatShrinkable.shrinksPtr]() {
            auto zero = Stream<Shrinkable<FLOATTYPE>>::one(make_shrinkable<FLOATTYPE>(0.0f));
            return zero.concat((*shrinksPtr)());
        });

        // shrink fraction within (0.0 and 0.5)
        floatShrinkable = floatShrinkable.andThen([](const Shrinkable<FLOATTYPE>& shr) {
            auto value = shr.get();
            int exp = 0;
            /*FLOATTYPE fraction = */ util::decomposeFloat(value, &exp);
            if (value == 0.0f)
                return Stream<Shrinkable<FLOATTYPE>>::empty();
            else if (value > 0) {
                return Stream<Shrinkable<FLOATTYPE>>::one(make_shrinkable<FLOATTYPE>(util::composeFloat(0.5f, exp)));
            } else {
                return Stream<Shrinkable<FLOATTYPE>>::one(make_shrinkable<FLOATTYPE>(util::composeFloat(-0.5f, exp)));
            }
        });

        // integerfy
        floatShrinkable = floatShrinkable.andThen([](const Shrinkable<FLOATTYPE>& shr) {
            auto value = shr.get();
            auto intValue = static_cast<int>(value);
            if (intValue != 0 && std::abs(intValue) < std::abs(value)) {
                return Stream<Shrinkable<FLOATTYPE>>::one(make_shrinkable<FLOATTYPE>(intValue));
            } else
                return Stream<Shrinkable<FLOATTYPE>>::empty();
        });

        return floatShrinkable.shrinks();
    }
}

Shrinkable<float> Arbitrary<float>::operator()(Random& rand)
{
    auto raw = rand.getRandomUInt32();
    float value = *reinterpret_cast<float*>(&raw);

    return make_shrinkable<float>(value).with([value]() { return shrinkFloat(value); });
}

Shrinkable<double> Arbitrary<double>::operator()(Random& rand)
{
    auto raw = rand.getRandomUInt64();
    double value = *reinterpret_cast<double*>(&raw);

    return make_shrinkable<double>(value).with([value]() { return shrinkFloat(value); });
    // double value = rand.getRandomDouble();
    // return make_shrinkable<double>(value).with([value]() {
    //     if(value == 0.0)
    //         return Stream<Shrinkable<double>>::empty();
    //     else
    //         return Stream<Shrinkable<double>>::one(make_shrinkable<double>(0.0));
    // });
}

constexpr char Arbitrary<char>::boundaryValues[];
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
}  // namespace pbt
