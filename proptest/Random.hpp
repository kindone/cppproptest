#pragma once

#include "api.hpp"
#include "util/std.hpp"

namespace proptest {

PROPTEST_API int64_t getCurrentTime();

class PROPTEST_API Random {
public:
    Random(uint64_t seed);
    Random(const Random& other);
    bool getRandomBool(double threshold = 0.5);
    int8_t getRandomInt8(int8_t min = INT8_MIN, int8_t max = INT8_MAX);
    uint8_t getRandomUInt8(uint8_t min = 0, uint8_t max = UINT8_MAX);
    int16_t getRandomInt16(int16_t min = INT16_MIN, int16_t max = INT16_MAX);
    uint16_t getRandomUInt16(uint16_t min = 0, uint16_t max = UINT16_MAX);
    int32_t getRandomInt32(int32_t min = INT32_MIN, int32_t max = INT32_MAX);
    uint32_t getRandomUInt32(uint32_t min = 0, uint32_t max = UINT32_MAX);
    int64_t getRandomInt64(int64_t min = INT64_MIN, int64_t max = INT64_MAX);
    uint64_t getRandomUInt64(uint64_t min = 0, uint64_t max = UINT64_MAX);
    float getRandomFloat();
    double getRandomDouble();
    float getRandomFloat(float min, float max);
    double getRandomDouble(double min, double max);
    uint32_t getRandomSize(size_t fromIncluded, size_t toExcluded);

    Random& operator=(const Random& other);

    template <typename T>
    T getRandom(int64_t /*min*/, int64_t /*max*/)
    {
        throw runtime_error("getRandom for type not defined");
    }

    template <typename T>
    T getRandomU(uint64_t /*min*/, uint64_t /*max*/)
    {
        throw runtime_error("getRandom for type not defined");
    }

private:
    uint64_t next8U();
    // default_random_engine engine;
    mt19937_64 engine;
    uniform_int_distribution<uint64_t> dist;
};

template <>
char Random::getRandom<char>(int64_t min, int64_t max);

template <>
int8_t Random::getRandom<int8_t>(int64_t min, int64_t max);

template <>
int16_t Random::getRandom<int16_t>(int64_t min, int64_t max);

template <>
int32_t Random::getRandom<int32_t>(int64_t min, int64_t max);

template <>
int64_t Random::getRandom<int64_t>(int64_t min, int64_t max);

template <>
char Random::getRandomU<char>(uint64_t min, uint64_t max);

template <>
uint8_t Random::getRandomU<uint8_t>(uint64_t min, uint64_t max);

template <>
uint16_t Random::getRandomU<uint16_t>(uint64_t min, uint64_t max);

template <>
uint32_t Random::getRandomU<uint32_t>(uint64_t min, uint64_t max);

template <>
uint64_t Random::getRandomU<uint64_t>(uint64_t min, uint64_t max);

}  // namespace proptest
