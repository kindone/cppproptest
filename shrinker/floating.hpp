#pragma once
#include "../Shrinkable.hpp"
#include "../generator/util.hpp"

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
Stream shrinkFloat(FLOATTYPE value)
{
    int exp = 0;
    if (value == 0.0f) {
        return Stream::empty();
    } else if (isnan(value)) {
        return Stream::one(make_shrinkable_any<FLOATTYPE>(0.0f));
    } else {
        FLOATTYPE fraction = 0.0f;
        if (isinf(value)) {
            if (value > 0) {
                auto max = numeric_limits<FLOATTYPE>::max();
                fraction = util::decomposeFloat(max, &exp);
            } else {
                auto min = numeric_limits<FLOATTYPE>::lowest();
                fraction = util::decomposeFloat(min, &exp);
            }
        } else {
            fraction = util::decomposeFloat(value, &exp);
        }

        auto expShrinkable = util::binarySearchShrinkable(exp);
        // shrink exponent
        auto floatShrinkable =
            expShrinkable.map<FLOATTYPE>([fraction](const int& exp) { return util::composeFloat(fraction, exp); });

        // prepend 0.0
        floatShrinkable = floatShrinkable.with([shrinksPtr = floatShrinkable.shrinksPtr]() {
            auto zero = Stream::one(make_shrinkable_any<FLOATTYPE>(0.0f));
            return zero.concat((*shrinksPtr)());
        });

        // shrink fraction within (0.0 and 0.5)
        floatShrinkable = floatShrinkable.andThen(+[](const Shrinkable<FLOATTYPE>& shr) {
            auto value = shr.get();
            int exp = 0;
            /*FLOATTYPE fraction = */ util::decomposeFloat(value, &exp);
            if (value == 0.0f)
                return Stream::empty();
            else if (value > 0) {
                return Stream::one(make_shrinkable_any<FLOATTYPE>(util::composeFloat(0.5f, exp)));
            } else {
                return Stream::one(make_shrinkable_any<FLOATTYPE>(util::composeFloat(-0.5f, exp)));
            }
        });

        // integerfy
        floatShrinkable = floatShrinkable.andThen(+[](const Shrinkable<FLOATTYPE>& shr) {
            auto value = shr.get();
            auto intValue = static_cast<int>(value);
            if (intValue != 0 && abs(intValue) < abs(value)) {
                return Stream::one(make_shrinkable_any<FLOATTYPE>(intValue));
            } else
                return Stream::empty();
        });

        return floatShrinkable.shrinks();
    }
}

} // namespace proptest
