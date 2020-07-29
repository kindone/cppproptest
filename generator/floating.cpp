#include "../gen.hpp"
#include "floating.hpp"
#include "bool.hpp"
#include "util.hpp"
#include "tuple.hpp"
#include <string>
#include <functional>
#include <cmath>

namespace proptest {

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

        auto expShrinkable = util::binarySearchShrinkable(exp);
        // shrink exponent
        auto floatShrinkable = expShrinkable.transform<FLOATTYPE>(
            [fraction](const int& exp) { return util::composeFloat(fraction, exp); });

        // prepend 0.0
        floatShrinkable = floatShrinkable.with([shrinksPtr = floatShrinkable.shrinksPtr]() {
            auto zero = Stream<Shrinkable<FLOATTYPE>>::one(make_shrinkable<FLOATTYPE>(0.0f));
            return zero.concat((*shrinksPtr)());
        });

        // shrink fraction within (0.0 and 0.5)
        floatShrinkable = floatShrinkable.andThen(+[](const Shrinkable<FLOATTYPE>& shr) {
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
        floatShrinkable = floatShrinkable.andThen(+[](const Shrinkable<FLOATTYPE>& shr) {
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
}

constexpr float Arbitrary<float>::boundaryValues[];
constexpr double Arbitrary<double>::boundaryValues[];

}  // namespace proptest
