#pragma once

#include "std.hpp"

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

}  // namespace proptest
