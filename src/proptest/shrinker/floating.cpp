#include "proptest/api.hpp"
#include "proptest/util/std.hpp"
#include "proptest/util/any.hpp"
#include "proptest/Stream.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/floating.hpp"

namespace proptest {

namespace util {

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

} // namespace util


// function instantiation
template Stream shrinkFloat<double>(double value);
template Stream shrinkFloat<float>(float value);



} // namespace proptest
