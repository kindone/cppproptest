#include "proptest/Random.hpp"
#include "proptest/util/std.hpp"

namespace proptest {

Random::Random(uint64_t seed) : engine(seed) {}

Random::Random(const Random& other) : engine(other.engine), dist(other.dist) {}

Random& Random::operator=(const Random& other)
{
    engine = other.engine;
    dist = other.dist;

    return *this;
}

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
    uniform_int_distribution<int> dist(min, max);
    return static_cast<int8_t>(dist(engine));
}

uint8_t Random::getRandomUInt8(uint8_t min, uint8_t max)
{
    uniform_int_distribution<int> dist(min, max);
    return static_cast<uint8_t>(dist(engine));
}

int16_t Random::getRandomInt16(int16_t min, int16_t max)
{
    uniform_int_distribution<int> dist(min, max);
    return static_cast<int16_t>(dist(engine));
}

uint16_t Random::getRandomUInt16(uint16_t min, uint16_t max)
{
    uniform_int_distribution<int> dist(min, max);
    return static_cast<uint16_t>(dist(engine));
}

int32_t Random::getRandomInt32(int32_t min, int32_t max)
{
    uniform_int_distribution<int32_t> dist(min, max);
    return static_cast<int32_t>(dist(engine));
}

uint32_t Random::getRandomUInt32(uint32_t min, uint32_t max)
{
    uniform_int_distribution<uint32_t> dist(min, max);
    return static_cast<uint32_t>(dist(engine));
}

int64_t Random::getRandomInt64(int64_t min, int64_t max)
{
    uniform_int_distribution<int64_t> dist(min, max);
    return static_cast<int64_t>(dist(engine));
}

uint64_t Random::getRandomUInt64(uint64_t min, uint64_t max)
{
    uniform_int_distribution<uint64_t> dist(min, max);
    return static_cast<uint64_t>(dist(engine));
}

// [fromIncluded, toExclued)
uint32_t Random::getRandomSize(size_t fromIncluded, size_t toExcluded)
{
    return getRandomUInt32(fromIncluded, toExcluded-1);
}

float Random::getRandomFloat()
{
    uniform_real_distribution<float> dist;
    return dist(engine);
}

double Random::getRandomDouble()
{
    uniform_real_distribution<double> dist;
    return dist(engine);
}
float Random::getRandomFloat(float min, float max)
{
    uniform_real_distribution<float> dist(min, max);
    return dist(engine);
}

double Random::getRandomDouble(double min, double max)
{
    uniform_real_distribution<double> dist(min, max);
    return dist(engine);
}


int64_t getCurrentTime()
{
    auto curTime = std::chrono::system_clock::now();
    auto duration = curTime.time_since_epoch();
    auto millis = duration_cast<util::milliseconds>(duration).count();
    return millis;
}

template <>
char Random::getRandom<char>(int64_t min, int64_t max)
{
    return static_cast<char>(getRandomUInt8(static_cast<uint8_t>(min), static_cast<uint8_t>(max)));
}

template <>
int8_t Random::getRandom<int8_t>(int64_t min, int64_t max)
{
    return getRandomInt8(static_cast<int8_t>(min), static_cast<int8_t>(max));
}

template <>
int16_t Random::getRandom<int16_t>(int64_t min, int64_t max)
{
    return getRandomInt16(static_cast<int16_t>(min), static_cast<int16_t>(max));
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
    return static_cast<char>(getRandomUInt8(static_cast<uint8_t>(min), static_cast<uint8_t>(max)));
}

template <>
uint8_t Random::getRandomU<uint8_t>(uint64_t min, uint64_t max)
{
    return getRandomUInt8(static_cast<uint8_t>(min), static_cast<uint8_t>(max));
}

template <>
uint16_t Random::getRandomU<uint16_t>(uint64_t min, uint64_t max)
{
    return getRandomUInt16(static_cast<uint16_t>(min), static_cast<uint16_t>(max));
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
