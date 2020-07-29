#include "../gen.hpp"
#include "integral.hpp"
#include "bool.hpp"
#include "util.hpp"
#include <string>
#include <functional>
#include <cmath>

namespace proptest {

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

constexpr char Arbitrary<char>::boundaryValues[];
constexpr int8_t Arbitrary<int8_t>::boundaryValues[];
constexpr int16_t Arbitrary<int16_t>::boundaryValues[];
constexpr int32_t Arbitrary<int32_t>::boundaryValues[];
constexpr int64_t Arbitrary<int64_t>::boundaryValues[];
constexpr uint8_t Arbitrary<uint8_t>::boundaryValues[];
constexpr uint16_t Arbitrary<uint16_t>::boundaryValues[];
constexpr uint32_t Arbitrary<uint32_t>::boundaryValues[];
constexpr uint64_t Arbitrary<uint64_t>::boundaryValues[];

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
}  // namespace proptest
