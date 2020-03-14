#ifndef TESTING_RANDOM_HPP
#define TESTING_RANDOM_HPP
#include <random>

namespace PropertyBasedTesting {

int64_t getCurrentTime();

class Random
{
public:
    Random(uint64_t seed);
    Random(const Random& other);
    bool getRandomBool();
    int8_t getRandomInt8();
    uint8_t getRandomUInt8();
    int16_t getRandomInt16();
    uint16_t getRandomUInt16();
    int32_t getRandomInt32();
    uint32_t getRandomUInt32();
    int64_t getRandomInt64();
    uint64_t getRandomUInt64();
    float getRandomFloat();
    double getRandomDouble();
    uint32_t getRandomSize(size_t fromIncluded, size_t toExcluded);
private:
    uint64_t next8U();
    //std::default_random_engine engine;
    std::mt19937_64 engine;
    std::uniform_int_distribution<uint64_t> dist;

};

} // namespace PropertyBasedTesting
#endif // TESTING_RANDOM_HPP

