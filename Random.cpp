#include "Random.hpp"
#include "util/std.hpp"

namespace proptest {

Random::Random(uint64_t seed) : engine(seed) {}

Random::Random(const Random& other) : engine(other.engine), dist(other.dist) {}

uint64_t Random::next8U()
{
    auto value = dist(engine);
    return value;
}

bool Random::getRandomBool(double threshold)
{
    if(threshold == 1.0)
        return true;
    return (next8U() <= static_cast<uint64_t>(static_cast<double>(UINT64_MAX) * threshold));
}

int8_t Random::getRandomInt8(int8_t min, int8_t max)
{
    uint64_t span = static_cast<uint64_t>(static_cast<int64_t>(max) - static_cast<int64_t>(min) + 1);
    uint8_t unsignedVal = (getRandomUInt8() % span);
    return *reinterpret_cast<int8_t*>(&unsignedVal) + min;
}

uint8_t Random::getRandomUInt8(uint8_t min, uint8_t max)
{
    uint64_t span = static_cast<uint64_t>(max) - static_cast<uint64_t>(min) + 1;
    return static_cast<uint8_t>((next8U() % span) + min);
}

int16_t Random::getRandomInt16(int16_t min, int16_t max)
{
    uint64_t span = static_cast<uint64_t>(static_cast<int64_t>(max) - static_cast<int64_t>(min) + 1);
    uint16_t unsignedVal = (getRandomUInt16() % span);
    return *reinterpret_cast<int16_t*>(&unsignedVal) + min;
}

uint16_t Random::getRandomUInt16(uint16_t min, uint16_t max)
{
    uint64_t span = static_cast<uint64_t>(max) - static_cast<uint64_t>(min) + 1;
    return static_cast<uint16_t>((next8U() % span) + min);
}

int32_t Random::getRandomInt32(int32_t min, int32_t max)
{
    uint64_t span = static_cast<uint64_t>(static_cast<int64_t>(max) - static_cast<int64_t>(min) + 1);
    uint32_t unsignedVal = (getRandomUInt32() % span);
    return *reinterpret_cast<int32_t*>(&unsignedVal) + min;
}

uint32_t Random::getRandomUInt32(uint32_t min, uint32_t max)
{
    uint64_t span = static_cast<uint64_t>(max) - static_cast<uint64_t>(min) + 1;
    return static_cast<uint32_t>((next8U() % span) + min);
}

int64_t Random::getRandomInt64(int64_t min, int64_t max)
{
    if (min == INT64_MIN && max == INT64_MAX) {
        uint64_t unsignedVal = getRandomUInt64();
        return *reinterpret_cast<int64_t*>(&unsignedVal) + INT64_MIN;
    }
    uint64_t span = max - min + 1;
    uint64_t unsignedVal = (getRandomUInt64() % span);
    return *reinterpret_cast<int64_t*>(&unsignedVal) + min;
}

uint64_t Random::getRandomUInt64(uint64_t min, uint64_t max)
{
    if (min == 0 && max == UINT64_MAX)
        return next8U();

    uint64_t span = max - min + 1;
    return static_cast<uint64_t>((next8U() % span) + min);
}

// [fromIncluded, toExclued)
uint32_t Random::getRandomSize(size_t fromIncluded, size_t toExcluded)
{
    return (next8U() % (toExcluded - fromIncluded)) + fromIncluded;
}

float Random::getRandomFloat()
{
    uint32_t intVal = getRandomUInt32();
    return *reinterpret_cast<float*>(&intVal);
}

double Random::getRandomDouble()
{
    uint64_t intVal = getRandomUInt64();
    return *reinterpret_cast<double*>(&intVal);
}

int64_t getCurrentTime()
{
    auto curTime = std::chrono::system_clock::now();
    auto duration = curTime.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    return millis;
}

template <>
char Random::getRandom<char>(int64_t min, int64_t max)
{
    return static_cast<char>(getRandomUInt8(min, max));
}

template <>
int8_t Random::getRandom<int8_t>(int64_t min, int64_t max)
{
    return getRandomInt8(min, max);
}

template <>
int16_t Random::getRandom<int16_t>(int64_t min, int64_t max)
{
    return getRandomInt16(min, max);
}

template <>
int32_t Random::getRandom<int32_t>(int64_t min, int64_t max)
{
    return getRandomInt32(min, max);
}

template <>
int64_t Random::getRandom<int64_t>(int64_t min, int64_t max)
{
    return getRandomInt64(min, max);
}

template <>
char Random::getRandomU<char>(uint64_t min, uint64_t max)
{
    return static_cast<char>(getRandomUInt8(min, max));
}

template <>
uint8_t Random::getRandomU<uint8_t>(uint64_t min, uint64_t max)
{
    return getRandomUInt8(min, max);
}

template <>
uint16_t Random::getRandomU<uint16_t>(uint64_t min, uint64_t max)
{
    return getRandomUInt16(min, max);
}

template <>
uint32_t Random::getRandomU<uint32_t>(uint64_t min, uint64_t max)
{
    return getRandomUInt32(min, max);
}

template <>
uint64_t Random::getRandomU<uint64_t>(uint64_t min, uint64_t max)
{
    return getRandomUInt64(min, max);
}
}  // namespace proptest
