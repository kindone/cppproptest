#pragma once
#include <exception>
#include <system_error>
#include <memory>
#include <iostream>
#include <sstream>

namespace proptest {

struct AssertFailed : public std::logic_error
{
    AssertFailed(const char* fname, int line, const std::error_code& /*error*/, const char* condition,
                 const void* /*caller*/)
        : logic_error(condition), filename(fname), lineno(line)
    {
    }

    const char* filename;
    int lineno;
};

struct PropertyFailedBase : public std::logic_error
{
    PropertyFailedBase(const AssertFailed& e) : logic_error(e), filename(e.filename), lineno(e.lineno) {}
    const char* filename;
    int lineno;
};

template <typename ValueTuple>
struct PropertyFailed : public PropertyFailedBase
{
    PropertyFailed(const AssertFailed& e, std::shared_ptr<ValueTuple> v) : PropertyFailedBase(e), valueTupPtr(v) {}
    std::shared_ptr<ValueTuple> valueTupPtr;
};

struct Discard : public std::logic_error
{
    Discard(const char* /*fname*/, int /*line*/, const std::error_code& /*error*/, const char* /*condition*/,
            const void* /*caller*/)
        : logic_error("Discard")
    {
    }
};

struct Success : public std::logic_error
{
    Success(const char* /*fname*/, int /*line*/, const std::error_code& /*error*/, const char* /*condition*/,
            const void* /*caller*/)
        : logic_error("Success")
    {
    }
};

namespace util {
std::ostream& errorOrEmpty(bool condition);
}

}  // namespace proptest

#define PROP_ASSERT_VARGS(condition, code)                                                                 \
    do {                                                                                                   \
        if (!(condition)) {                                                                                \
            ::proptest::AssertFailed __proptest_except_obj(__FILE__, __LINE__, code, #condition, nullptr); \
            throw __proptest_except_obj;                                                                   \
        }                                                                                                  \
    } while (false)

#define PROP_ASSERT_STREAM(condition, a, sign, b)                                 \
    do {                                                                          \
        if (!(condition)) {                                                       \
            std::stringstream __prop_assert_stream_str;                           \
            __prop_assert_stream_str << #condition << " with " << a << sign << b; \
            PROP_ASSERT_VARGS(__prop_assert_stream_str, {});                      \
        }                                                                         \
    } while (false)

#define PROP_ASSERT(condition) PROP_ASSERT_VARGS(condition, {})
#define PROP_ASSERT_TRUE(condition) PROP_ASSERT_VARGS(condition, {})
#define PROP_ASSERT_FALSE(condition) PROP_ASSERT_VARGS(!condition, {})

#define PROP_ASSERT_EQ(a, b) PROP_ASSERT_STREAM(a == b, a, " != ", b)
#define PROP_ASSERT_NE(a, b) PROP_ASSERT_STREAM(a != b, a, " == ", b)
#define PROP_ASSERT_LT(a, b) PROP_ASSERT_STREAM(a < b, a, " >= ", b)
#define PROP_ASSERT_GT(a, b) PROP_ASSERT_STREAM(a > b, a, " <= ", b)
#define PROP_ASSERT_LE(a, b) PROP_ASSERT_STREAM(a <= b, a, " > ", b)
#define PROP_ASSERT_GE(a, b) PROP_ASSERT_STREAM(a >= b, a, " < ", b)

#define PROP_ASSERT_STREQ(a, b, n) \
    PROP_ASSERT_STREAM(memcmp(a, b, n) == 0, proptest::Show<char*>(a, n), " not equals ", proptest::Show<char*>(b, n))

#define PROP_ASSERT_STRNE(a, b, n) \
    PROP_ASSERT_STREAM(memcmp(a, b, n) != 0, proptest::Show<char*>(a, n), " equals ", proptest::Show<char*>(b, n))

#define PROP_DISCARD()                                                  \
    do {                                                                \
        throw ::proptest::Discard(__FILE__, __LINE__, {}, "", nullptr); \
    } while (false)

#define PROP_SUCCESS()                                                  \
    do {                                                                \
        throw ::proptest::Success(__FILE__, __LINE__, {}, "", nullptr); \
    } while (false)
