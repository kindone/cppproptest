#pragma once

#include "proptest/api.hpp"
#include "proptest/gen.hpp"
#include "proptest/PropertyContext.hpp"
#include "proptest/util/std.hpp"

#define PROP_EXPECT_STREAM(condition, a, sign, b)                                            \
    ([&]() -> stringstream& {                                                           \
        if (!(condition)) {                                                                  \
            stringstream __prop_expect_stream_str;                                      \
            __prop_expect_stream_str << (a) << (sign) << (b);                                      \
            ::proptest::PropertyBase::fail(__FILE__, __LINE__, #condition, __prop_expect_stream_str);    \
        } else {                                                                             \
            stringstream __prop_expect_stream_str;                                      \
            ::proptest::PropertyBase::succeed(__FILE__, __LINE__, #condition, __prop_expect_stream_str); \
        }                                                                                    \
        return ::proptest::PropertyBase::getLastStream();                                                \
    })()

#define PROP_EXPECT(cond) PROP_EXPECT_STREAM(cond, "", "", "")
#define PROP_EXPECT_TRUE(cond) PROP_EXPECT_STREAM(cond, "", "", "")
#define PROP_EXPECT_FALSE(cond) PROP_EXPECT_STREAM(cond, cond, " == ", "true")
#define PROP_EXPECT_EQ(a, b) PROP_EXPECT_STREAM((a == b), a, " != ", b)
#define PROP_EXPECT_NE(a, b) PROP_EXPECT_STREAM((a != b), a, " == ", b)
#define PROP_EXPECT_LT(a, b) PROP_EXPECT_STREAM((a < b), a, " >= ", b)
#define PROP_EXPECT_GT(a, b) PROP_EXPECT_STREAM((a > b), a, " <= ", b)
#define PROP_EXPECT_LE(a, b) PROP_EXPECT_STREAM((a <= b), a, " > ", b)
#define PROP_EXPECT_GE(a, b) PROP_EXPECT_STREAM((a >= b), a, " < ", b)
#define PROP_EXPECT_STREQ(a, b, n) \
    PROP_EXPECT_STREAM(memcmp(a, b, n) == 0, ::proptest::Show<char*>(a, n), " not equals ", ::proptest::Show<char*>(b, n))
#define PROP_EXPECT_STREQ2(a, b, n1, n2)                                                                      \
    PROP_EXPECT_STREAM(memcmp(a, b, (n1 <= n2 ? n1 : n2)) == 0, ::proptest::Show<char*>(a, n1), " not equals ", \
                       ::proptest::Show<char*>(b, n2))
#define PROP_EXPECT_STRNE(a, b, n) \
    PROP_EXPECT_STREAM(memcmp(a, b, n) != 0, ::proptest::Show<char*>(a, n), " equals ", ::proptest::Show<char*>(b, n))
#define PROP_EXPECT_STRNE2(a, b, n1, n2)                                                                  \
    PROP_EXPECT_STREAM(memcmp(a, b, (n1 <= n2 ? n1 : n2)) != 0, ::proptest::Show<char*>(a, n1), " equals ", \
                       ::proptest::Show<char*>(b, n2))

#define PROP_STAT(VALUE)                                                                       \
    do {                                                                                       \
        stringstream __prop_stat_key;                                                     \
        __prop_stat_key << (#VALUE);                                                           \
        stringstream __prop_stat_value;                                                   \
        __prop_stat_value << boolalpha;                                                   \
        __prop_stat_value << (VALUE);                                                          \
        ::proptest::PropertyBase::tag(__FILE__, __LINE__, __prop_stat_key.str(), __prop_stat_value.str()); \
    } while (false)

#define PROP_TAG(KEY, VALUE)                                                                   \
    do {                                                                                       \
        stringstream __prop_stat_key;                                                     \
        __prop_stat_key << (KEY);                                                              \
        stringstream __prop_stat_value;                                                   \
        __prop_stat_value << boolalpha;                                                   \
        __prop_stat_value << (VALUE);                                                          \
        ::proptest::PropertyBase::tag(__FILE__, __LINE__, __prop_stat_key.str(), __prop_stat_value.str()); \
    } while (false)

#define PROP_CLASSIFY(condition, KEY, VALUE)                   \
    do {                                                       \
        if (condition) {                                       \
            ::proptest::PropertyBase::tag(__FILE__, __LINE__, KEY, VALUE); \
        }                                                      \
    } while (false)

namespace proptest {

class Random;

class PROPTEST_API PropertyBase {
public:
    template <typename Func, typename GenTuple>
    PropertyBase(Func* _funcPtr, GenTuple* _genTupPtr)
 : seed(util::getGlobalSeed()), numRuns(defaultNumRuns), maxDurationMs(defaultMaxDurationMs), funcPtr(_funcPtr), genTupPtr(_genTupPtr)  {}

    static void setDefaultNumRuns(uint32_t);
    static void tag(const char* filename, int lineno, string key, string value);
    static void succeed(const char* filename, int lineno, const char* condition, const stringstream& str);
    static void fail(const char* filename, int lineno, const char* condition, const stringstream& str);
    static stringstream& getLastStream();

protected:
    static void setContext(PropertyContext* context);
    static PropertyContext* getContext() { return context; }
    static PropertyContext* context;

protected:
    bool invoke(Random& rand);

    static uint32_t defaultNumRuns;
    static uint32_t defaultMaxDurationMs;

    // TODO: configurations
    uint64_t seed;
    uint32_t numRuns;

    uint32_t maxDurationMs; // indefinitely if 0

    shared_ptr<void> funcPtr;
    shared_ptr<void> genTupPtr;
    shared_ptr<function<void()>> onStartupPtr;
    shared_ptr<function<void()>> onCleanupPtr;

    friend struct PropertyContext;
};

}  // namespace proptest
