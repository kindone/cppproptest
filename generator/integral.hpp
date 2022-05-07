#pragma once
#include "../gen.hpp"
#include "../api.hpp"
#include "../Stream.hpp"
#include "util.hpp"

/**
 * @file integral.hpp
 * @brief Arbitrary for integers
 */

namespace proptest {

template <typename GEN>
decltype(auto) generator(GEN&& gen);

template <typename T>
Shrinkable<T> generateInteger(Random& rand, T min = numeric_limits<T>::min(), T max = numeric_limits<T>::max())
{
    T value = 0;
    if (min == numeric_limits<T>::min() && max == numeric_limits<T>::max() && rand.getRandomBool()) {
        uint32_t i = rand.getRandomSize(0, sizeof(Arbi<T>::boundaryValues) / sizeof(Arbi<T>::boundaryValues[0]));
        value = Arbi<T>::boundaryValues[i];
    } else if (numeric_limits<T>::min() < 0)
        value = rand.getRandom<T>(min, max);
    else
        value = rand.getRandomU<T>(min, max);

    if (value < min || max < value)
        throw runtime_error("invalid range");

    if (min >= 0)  // [3,5] -> [0,2] -> [3,5]
    {
        return util::binarySearchShrinkableU(static_cast<T>(value - min))
            .template map<T>([min](const uint64_t& _value) { return static_cast<T>(_value + min); });
    } else if (max <= 0)  // [-5,-3] -> [-2,0] -> [-5,-3]
    {
        return util::binarySearchShrinkable(static_cast<T>(value - max)).template map<T>([max](const int64_t& _value) {
            return static_cast<T>(_value + max);
        });
    } else  // [-2, 2]
    {
        auto transformer = +[](const int64_t& _value) { return static_cast<T>(_value); };
        return util::binarySearchShrinkable(value).template map<T>(transformer);
    }
}

/**
 * @ingroup Generators
 * @brief Arbitrary for int8_t
 */
template <>
class PROPTEST_API Arbi<int8_t> final : public ArbiBase<int8_t> {
public:
    Shrinkable<int8_t> operator()(Random& rand) override;
    static constexpr int8_t boundaryValues[] = {INT8_MIN,     0,   INT8_MAX, -1,   1,    -2,   2,   INT8_MIN + 1,
                                                INT8_MAX - 1, ' ', '"',      '\'', '\t', '\n', '\r'};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for int16_t
 */
template <>
class PROPTEST_API Arbi<int16_t> final : public ArbiBase<int16_t> {
public:
    Shrinkable<int16_t> operator()(Random& rand) override;
    static constexpr int16_t boundaryValues[] = {0,
                                                 -1,
                                                 1,
                                                 -2,
                                                 2,
                                                 INT16_MIN,
                                                 INT16_MAX,
                                                 INT16_MIN + 1,
                                                 INT16_MAX - 1,
                                                 INT8_MIN,
                                                 INT8_MAX,
                                                 UINT8_MAX,
                                                 INT8_MIN - 1,
                                                 INT8_MIN + 1,
                                                 INT8_MAX - 1,
                                                 INT8_MAX + 1,
                                                 UINT8_MAX - 1,
                                                 UINT8_MAX + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for int32_t
 */
template <>
struct PROPTEST_API Arbi<int32_t> final : public ArbiBase<int32_t>
{
public:
    Shrinkable<int32_t> operator()(Random& rand) override;
    static constexpr int32_t boundaryValues[] = {0,
                                                 -1,
                                                 1,
                                                 -2,
                                                 2,
                                                 INT32_MIN,
                                                 INT32_MAX,
                                                 INT32_MIN + 1,
                                                 INT32_MAX - 1,
                                                 INT16_MIN,
                                                 INT16_MAX,
                                                 UINT16_MAX,
                                                 INT16_MIN - 1,
                                                 INT16_MIN + 1,
                                                 INT16_MAX - 1,
                                                 INT16_MAX + 1,
                                                 UINT16_MAX - 1,
                                                 UINT16_MAX + 1,
                                                 INT8_MIN,
                                                 INT8_MAX,
                                                 UINT8_MAX,
                                                 INT8_MIN - 1,
                                                 INT8_MIN + 1,
                                                 INT8_MAX - 1,
                                                 INT8_MAX + 1,
                                                 UINT8_MAX - 1,
                                                 UINT8_MAX + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for int64_t
 */
template <>
struct PROPTEST_API Arbi<int64_t> final : public ArbiBase<int64_t>
{
public:
    Shrinkable<int64_t> operator()(Random& rand) override;
    static constexpr int64_t boundaryValues[] = {0,
                                                 -1,
                                                 1,
                                                 -2,
                                                 2,
                                                 INT64_MIN,
                                                 INT64_MAX,
                                                 INT64_MIN + 1,
                                                 INT64_MAX - 1,
                                                 INT32_MIN,
                                                 INT32_MAX,
                                                 UINT32_MAX,
                                                 static_cast<int64_t>(INT32_MIN) - 1,
                                                 INT32_MIN + 1,
                                                 INT32_MAX - 1,
                                                 static_cast<int64_t>(INT32_MAX) + 1,
                                                 UINT32_MAX - 1,
                                                 static_cast<int64_t>(UINT32_MAX) + 1,
                                                 INT16_MIN,
                                                 INT16_MAX,
                                                 UINT16_MAX,
                                                 INT16_MIN - 1,
                                                 INT16_MIN + 1,
                                                 INT16_MAX - 1,
                                                 INT16_MAX + 1,
                                                 UINT16_MAX - 1,
                                                 UINT16_MAX + 1,
                                                 INT8_MIN,
                                                 INT8_MAX,
                                                 UINT8_MAX,
                                                 INT8_MIN - 1,
                                                 INT8_MIN + 1,
                                                 INT8_MAX - 1,
                                                 INT8_MAX + 1,
                                                 UINT8_MAX - 1,
                                                 UINT8_MAX + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for char
 */
template <>
class PROPTEST_API Arbi<char> final : public ArbiBase<char> {
public:
    Shrinkable<char> operator()(Random& rand) override;
    static constexpr char boundaryValues[] = {0};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for uint8_t
 */
template <>
class PROPTEST_API Arbi<uint8_t> final : public ArbiBase<uint8_t> {
public:
    Shrinkable<uint8_t> operator()(Random& rand) override;
    static constexpr uint8_t boundaryValues[] = {
        0, 1, 2, UINT8_MAX, UINT8_MAX - 1, INT8_MAX, INT8_MAX - 1, INT8_MAX + 1, ' ', '"', '\'', '\t', '\n', '\r'};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for uint16_t
 */
template <>
class PROPTEST_API Arbi<uint16_t> final : public ArbiBase<uint16_t> {
public:
    Shrinkable<uint16_t> operator()(Random& rand) override;
    static constexpr uint16_t boundaryValues[] = {0,
                                                  1,
                                                  2,
                                                  UINT16_MAX,
                                                  UINT16_MAX - 1,
                                                  INT16_MAX,
                                                  INT16_MAX - 1,
                                                  INT16_MAX + 1,
                                                  INT8_MAX,
                                                  UINT8_MAX,
                                                  INT8_MAX + 1,
                                                  INT8_MAX - 1,
                                                  UINT8_MAX - 1,
                                                  UINT8_MAX + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for uint32_t
 */
template <>
struct PROPTEST_API Arbi<uint32_t> final : public ArbiBase<uint32_t>
{
public:
    Shrinkable<uint32_t> operator()(Random& rand) override;
    static constexpr uint32_t boundaryValues[] = {0,
                                                  1,
                                                  2,
                                                  UINT32_MAX,
                                                  UINT32_MAX - 1,
                                                  INT32_MAX,
                                                  INT32_MAX - 1,
                                                  static_cast<uint32_t>(INT32_MAX) + 1,
                                                  INT16_MAX,
                                                  UINT16_MAX,
                                                  INT16_MAX - 1,
                                                  INT16_MAX + 1,
                                                  UINT16_MAX - 1,
                                                  UINT16_MAX + 1,
                                                  INT8_MAX,
                                                  UINT8_MAX,
                                                  INT8_MAX + 1,
                                                  INT8_MAX - 1,
                                                  UINT8_MAX - 1,
                                                  UINT8_MAX + 1};
};

/**
 * @ingroup Generators
 * @brief Arbitrary for uint64_t
 */
template <>
struct PROPTEST_API Arbi<uint64_t> : public ArbiBase<uint64_t>
{
public:
    Shrinkable<uint64_t> operator()(Random& rand) override;
    static constexpr uint64_t boundaryValues[] = {0,
                                                  1,
                                                  2,
                                                  UINT64_MAX,
                                                  UINT64_MAX - 1,
                                                  INT32_MAX,
                                                  UINT32_MAX,
                                                  INT32_MAX - 1,
                                                  static_cast<int64_t>(INT32_MAX) + 1,
                                                  UINT32_MAX - 1,
                                                  static_cast<int64_t>(UINT32_MAX) + 1,
                                                  INT16_MAX,
                                                  UINT16_MAX,
                                                  INT16_MAX - 1,
                                                  INT16_MAX + 1,
                                                  UINT16_MAX - 1,
                                                  UINT16_MAX + 1,
                                                  INT8_MAX,
                                                  UINT8_MAX,
                                                  INT8_MAX + 1,
                                                  INT8_MAX - 1,
                                                  UINT8_MAX - 1,
                                                  UINT8_MAX + 1};
};

/**
 * @ingroup Generators
 * @brief Generates a positive integer, excluding 0
 */
template <typename T>
Generator<T> natural(T max = numeric_limits<T>::max())
{
    return Generator<T>([max](Random& rand) { return generateInteger<T>(rand, 1, max); });
}

/**
 * @ingroup Generators
 * @brief Generates 0 or a positive integer
 */
template <typename T>
Generator<T> nonNegative(T max = numeric_limits<T>::max())
{
    return Generator<T>([max](Random& rand) { return generateInteger<T>(rand, 0, max); });
}

/**
 * @ingroup Generators
 * @brief Generates numeric values in [min, max]. e.g. interval(0,100) generates a value in {0, ..., 100}
 */
template <typename T>
Generator<T> interval(T min, T max)
{
    return Generator<T>([min, max](Random& rand) { return generateInteger<T>(rand, min, max); });
}

/**
 * @ingroup Generators
 * @brief Generates numeric values in [from, to). e.g. inRange(0,100) generates a value in {0, ..., 99}
 */
template <typename T>
Generator<T> inRange(T from, T to)
{
    return Generator<T>([from, to](Random& rand) { return generateInteger<T>(rand, from, to - 1); });
}

/**
 * @ingroup Generators
 * @brief Generates numeric values in [a, a+count). e.g. integers(0,100) generates a value in {0, ..., 99}
 */
template <typename T>
Generator<T> integers(T start, T count)
{
    return Generator<T>(
        [start, count](Random& rand) { return generateInteger<T>(rand, start, static_cast<T>(start + count - 1)); });
}

}  // namespace proptest
