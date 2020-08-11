#include "../gen.hpp"
#include "integral.hpp"
#include "bool.hpp"
#include "util.hpp"
#include <string>
#include <functional>
#include <cmath>

namespace proptest {

Shrinkable<char> Arbi<char>::operator()(Random& rand)
{
    return generateInteger<char>(rand);
}

Shrinkable<int8_t> Arbi<int8_t>::operator()(Random& rand)
{
    return generateInteger<int8_t>(rand);
}

Shrinkable<int16_t> Arbi<int16_t>::operator()(Random& rand)
{
    return generateInteger<int16_t>(rand);
}

Shrinkable<int32_t> Arbi<int32_t>::operator()(Random& rand)
{
    return generateInteger<int32_t>(rand);
}

Shrinkable<int64_t> Arbi<int64_t>::operator()(Random& rand)
{
    return generateInteger<int64_t>(rand);
}

Shrinkable<uint8_t> Arbi<uint8_t>::operator()(Random& rand)
{
    return generateInteger<uint8_t>(rand);
}

Shrinkable<uint16_t> Arbi<uint16_t>::operator()(Random& rand)
{
    return generateInteger<uint16_t>(rand);
}

Shrinkable<uint32_t> Arbi<uint32_t>::operator()(Random& rand)
{
    return generateInteger<uint32_t>(rand);
}

Shrinkable<uint64_t> Arbi<uint64_t>::operator()(Random& rand)
{
    return generateInteger<uint64_t>(rand);
}

constexpr char Arbi<char>::boundaryValues[];
constexpr int8_t Arbi<int8_t>::boundaryValues[];
constexpr int16_t Arbi<int16_t>::boundaryValues[];
constexpr int32_t Arbi<int32_t>::boundaryValues[];
constexpr int64_t Arbi<int64_t>::boundaryValues[];
constexpr uint8_t Arbi<uint8_t>::boundaryValues[];
constexpr uint16_t Arbi<uint16_t>::boundaryValues[];
constexpr uint32_t Arbi<uint32_t>::boundaryValues[];
constexpr uint64_t Arbi<uint64_t>::boundaryValues[];

/*
template struct Arbi<int8_t>;
template struct Arbi<int16_t>;
template struct Arbi<int32_t>;
template struct Arbi<int64_t>;
template struct Arbi<uint8_t>;
template struct Arbi<uint16_t>;
template struct Arbi<uint32_t>;
template struct Arbi<uint64_t>;
template struct Arbi<float>;
template struct Arbi<double>;
*/
}  // namespace proptest
