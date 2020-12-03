#pragma once
#include "api.hpp"
#include <exception>
#include <system_error>
#include <memory>
#include <iostream>
#include <sstream>

namespace proptest {

struct PROPTEST_API AssertFailed : public std::logic_error
{
    AssertFailed(const char* fname, int line, const std::error_code& /*error*/, const char* condition,
                 const void* /*caller*/)
        : logic_error(condition), filename(fname), lineno(line)
    {
    }

    virtual ~AssertFailed();

    const char* filename;
    int lineno;
};

struct PROPTEST_API PropertyFailedBase : public std::logic_error
{
    PropertyFailedBase(const AssertFailed& e) : logic_error(e), filename(e.filename), lineno(e.lineno) {}
    virtual ~PropertyFailedBase();

    const char* filename;
    int lineno;
};

template <typename ValueTuple>
struct PROPTEST_API PropertyFailed : public PropertyFailedBase
{
    PropertyFailed(const AssertFailed& e) : PropertyFailedBase(e) {}
    virtual ~PropertyFailed();
};

template <typename ValueTuple>
PropertyFailed<ValueTuple>::~PropertyFailed()
{
}

struct PROPTEST_API Discard : public std::logic_error
{
    Discard(const char* /*fname*/, int /*line*/, const std::error_code& /*error*/, const char* /*condition*/,
            const void* /*caller*/)
        : logic_error("Discard")
    {
    }
    virtual ~Discard();
};

struct PROPTEST_API Success : public std::logic_error
{
    Success(const char* /*fname*/, int /*line*/, const std::error_code& /*error*/, const char* /*condition*/,
            const void* /*caller*/)
        : logic_error("Success")
    {
    }
    virtual ~Success();
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

#define PROP_ASSERT_STREAM(condition, a, sign, b)                                                                  \
    do {                                                                                                           \
        if (!(condition)) {                                                                                        \
            std::stringstream __prop_assert_stream_str;                                                            \
            __prop_assert_stream_str << #condition << " with " << a << sign << b;                                  \
            ::proptest::AssertFailed __proptest_except_obj(__FILE__, __LINE__, {}, __prop_assert_stream_str.str(), \
                                                           nullptr);                                               \
            throw __proptest_except_obj;                                                                           \
        }                                                                                                          \
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

#define PROP_ASSERT_STREQ(a, b, n)                                                                            \
    do {                                                                                                      \
        if (!(memcmp(a, b, n) == 0)) {                                                                        \
            std::stringstream __prop_assert_stream_str;                                                       \
            __prop_assert_stream_str << #a << " not equals " << #b << " with " << proptest::Show<char*>(a, n) \
                                     << " not equals " << proptest::Show<char*>(b, n);                        \
            ::proptest::AssertFailed __proptest_except_obj(__FILE__, __LINE__, {},                            \
                                                           __prop_assert_stream_str.str().c_str(), nullptr);  \
            throw __proptest_except_obj;                                                                      \
        }                                                                                                     \
    } while (false)

#define PROP_ASSERT_STREQ2(a, b, n1, n2)                                                                       \
    do {                                                                                                       \
        if (!(memcmp(a, b, (n1 <= n2 ? n1 : n2)) == 0)) {                                                      \
            std::stringstream __prop_assert_stream_str;                                                        \
            __prop_assert_stream_str << #a << " not equals " << #b << " with " << proptest::Show<char*>(a, n1) \
                                     << " not equals " << proptest::Show<char*>(b, n2);                        \
            ::proptest::AssertFailed __proptest_except_obj(__FILE__, __LINE__, {},                             \
                                                           __prop_assert_stream_str.str().c_str(), nullptr);   \
            throw __proptest_except_obj;                                                                       \
        }                                                                                                      \
    } while (false)

#define PROP_ASSERT_STRNE(a, b, n)                                                                           \
    do {                                                                                                     \
        if (!(memcmp(a, b, n) != 0)) {                                                                       \
            std::stringstream __prop_assert_stream_str;                                                      \
            __prop_assert_stream_str << #a << " equals " << #b << " with " << proptest::Show<char*>(a, n)    \
                                     << " equals " << proptest::Show<char*>(b, n);                           \
            ::proptest::AssertFailed __proptest_except_obj(__FILE__, __LINE__, {},                           \
                                                           __prop_assert_stream_str.str().c_str(), nullptr); \
            throw __proptest_except_obj;                                                                     \
        }                                                                                                    \
    } while (false)

#define PROP_ASSERT_STRNE2(a, b, n1, n2)                                                                     \
    do {                                                                                                     \
        if (!(memcmp(a, b, (n1 <= n2 ? n1 : n2)) != 0)) {                                                    \
            std::stringstream __prop_assert_stream_str;                                                      \
            __prop_assert_stream_str << #a << " equals " << #b << " with " << proptest::Show<char*>(a, n1)   \
                                     << " equals " << proptest::Show<char*>(b, n2);                          \
            ::proptest::AssertFailed __proptest_except_obj(__FILE__, __LINE__, {},                           \
                                                           __prop_assert_stream_str.str().c_str(), nullptr); \
            throw __proptest_except_obj;                                                                     \
        }                                                                                                    \
    } while (false)

#define PROP_DISCARD()                                                  \
    do {                                                                \
        throw ::proptest::Discard(__FILE__, __LINE__, {}, "", nullptr); \
    } while (false)

#define PROP_SUCCESS()                                                  \
    do {                                                                \
        throw ::proptest::Success(__FILE__, __LINE__, {}, "", nullptr); \
    } while (false)
