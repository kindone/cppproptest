#pragma once

#include "api.hpp"
#include "gen.hpp"
#include "PropertyContext.hpp"
#include <sstream>

#define PROP_EXPECT_STREAM(condition, a, sign, b)                       \
    ([&]() -> std::stringstream& {                                      \
        if (!(condition)) {                                             \
            std::stringstream str;                                      \
            str << a << sign << b;                                      \
            PropertyBase::fail(__FILE__, __LINE__, #condition, str);    \
        } else {                                                        \
            std::stringstream str;                                      \
            PropertyBase::succeed(__FILE__, __LINE__, #condition, str); \
        }                                                               \
        return PropertyBase::getLastStream();                           \
    })()

#define PROP_EXPECT(cond) PROP_EXPECT_STREAM(cond, "", "", "")
#define PROP_EXPECT_TRUE(cond) PROP_EXPECT_STREAM(cond, "", "", "")
#define PROP_EXPECT_FALSE(cond) PROP_EXPECT_STREAM(cond, cond, " == ", "true")
#define PROP_EXPECT_EQ(a, b) PROP_EXPECT_STREAM(a == b, a, " != ", b)
#define PROP_EXPECT_NE(a, b) PROP_EXPECT_STREAM(a != b, a, " == ", b)
#define PROP_EXPECT_LT(a, b) PROP_EXPECT_STREAM(a < b, a, " >= ", b)
#define PROP_EXPECT_GT(a, b) PROP_EXPECT_STREAM(a > b, a, " <= ", b)
#define PROP_EXPECT_LE(a, b) PROP_EXPECT_STREAM(a <= b, a, " > ", b)
#define PROP_EXPECT_GE(a, b) PROP_EXPECT_STREAM(a >= b, a, " < ", b)
#define PROP_EXPECT_STREQ(a, b, n)                                                                    \
    PROP_EXPECT_STREAM(memcmp(a, b, n) == 0, pbt::Show<char*>(a, n), " not equals ", \
                       pbt::Show<char*>(b, n))
#define PROP_EXPECT_STRNE(a, b, n)                                                                \
    PROP_EXPECT_STREAM(memcmp(a, b, n) != 0, pbt::Show<char*>(a, n), " equals ", \
                       pbt::Show<char*>(b, n))

#define PROP_STAT(VALUE)                                               \
    do {                                                               \
        std::stringstream key;                                         \
        key << (#VALUE);                                               \
        std::stringstream value;                                       \
        value << std::boolalpha;                                       \
        value << (VALUE);                                              \
        PropertyBase::tag(__FILE__, __LINE__, key.str(), value.str()); \
    } while (false);

#define PROP_TAG(KEY, VALUE)                                           \
    do {                                                               \
        std::stringstream key;                                         \
        key << (KEY);                                                  \
        std::stringstream value;                                       \
        value << std::boolalpha;                                       \
        value << (VALUE);                                              \
        PropertyBase::tag(__FILE__, __LINE__, key.str(), value.str()); \
    } while (false);

#define PROP_CLASSIFY(condition, KEY, VALUE)                   \
    do {                                                       \
        if (condition) {                                       \
            PropertyBase::tag(__FILE__, __LINE__, KEY, VALUE); \
        }                                                      \
    } while (false);

namespace pbt {

class Random;

class PROPTEST_API PropertyBase {
public:
    PropertyBase();
    bool forAll();
    virtual ~PropertyBase() {}
    static void setDefaultNumRuns(uint32_t);
    static void tag(const char* filename, int lineno, std::string key, std::string value);
    static void succeed(const char* filename, int lineno, const char* condition, const std::stringstream& str);
    static void fail(const char* filename, int lineno, const char* condition, const std::stringstream& str);
    static std::stringstream& getLastStream();

protected:
    static void setContext(PropertyContext* context);
    static PropertyContext* getContext() { return context; }
    static PropertyContext* context;

protected:
    virtual bool invoke(Random& rand) = 0;
    virtual void handleShrink(Random& savedRand /*, const PropertyFailedBase& e*/) = 0;

    static uint32_t defaultNumRuns;

    // TODO: configurations
    uint64_t seed;
    uint32_t numRuns;

    friend struct PropertyContext;
};

}  // namespace pbt
