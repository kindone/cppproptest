#ifndef TESTING_PROPERTY_ASSERT_HPP
#define TESTING_PROPERTY_ASSERT_HPP

#include <exception>
#include <system_error>

namespace PropertyBasedTesting {

struct AssertFailed : public std::logic_error {
    AssertFailed(const char* fname,
                int line,
                const std::error_code& error,
                const char* condition,
                const void* caller)
    : logic_error(condition) {
    }
};

struct PropertyFailedBase : public std::logic_error {
    PropertyFailedBase(const std::logic_error& e) : logic_error(e) {
    }
};

template <typename ValueTuple>
struct PropertyFailed : public PropertyFailedBase {
    PropertyFailed(const std::logic_error& e, ValueTuple&& v) : PropertyFailedBase(e), valueTup(std::move(v)) {
    }

    ValueTuple valueTup;
};


struct Discard : public std::logic_error {
    Discard(const char* fname,
                int line,
                const std::error_code& error,
                const char* condition,
                const void* caller)
    : logic_error("Discard") {
    }
};

struct Success : public std::logic_error {
    Success(const char* fname,
                int line,
                const std::error_code& error,
                const char* condition,
                const void* caller)
    : logic_error("Success") {
    }
};

}

#define PROP_ASSERT_VARGS(condition, code, /*args*/ ...)                                                                \
    do                                                                                                                  \
    {                                                                                                                   \
        if (!(condition))                                                                                 \
        {                                                                                                               \
            ::PropertyBasedTesting::AssertFailed __proptest_except_obj(__FILE__, __LINE__, code, #condition, nullptr);  \
            throw __proptest_except_obj;                                                                                \
        }                                                                                                               \
    } while (false)

#define PROP_ASSERT(condition, code) \
    PROP_ASSERT_VARGS(condition, code)

#define PROP_DISCARD() \
    do                                                              \
    {                                                               \
        throw ::PropertyBasedTesting::Discard(__FILE__, __LINE__, {}, "", nullptr);  \
    } while (false)

#define PROP_SUCCESS() \
    do                                                              \
    {                                                               \
        throw ::PropertyBasedTesting::Success(__FILE__, __LINE__, {}, "", nullptr);  \
    } while (false)


#endif // TESTING_PROPERTY_ASSERT_HPP
