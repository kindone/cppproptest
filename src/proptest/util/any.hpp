#pragma once

#include "proptest/util/std.hpp"

namespace proptest
{

struct Any {

    Any() = default;

    template <typename T>
    Any(const T& t) {
        ptr = static_pointer_cast<void>(util::make_shared<T>(t));
    }

    Any(const Any& other) : ptr(other.ptr) {}

    template <typename T>
    T& cast() const {
        return *static_pointer_cast<T>(ptr);
    }

    shared_ptr<void> ptr;
};

namespace util {

template <typename T, typename... Args>
Any make_any(Args&&... args)
{
    Any any;
    any.ptr = static_pointer_cast<void>(util::make_shared<T>(args...));
    return any;
}

template <typename T>
shared_ptr<Any> make_shared_any(const shared_ptr<T>& ptr)
{
    auto anyPtr = util::make_shared<Any>();
    anyPtr->ptr = static_pointer_cast<void>(ptr);
    return anyPtr;
}

}


}  // namespace proptest
