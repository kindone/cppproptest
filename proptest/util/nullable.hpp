#pragma once

#include "std.hpp"

namespace proptest {

template <typename T>
struct Nullable
{
    Nullable() {}
    Nullable(shared_ptr<T> _ptr) : ptr(_ptr) {}

    bool isNull() const { return !static_cast<bool>(ptr); }

    shared_ptr<T> ptr;
};

}  // namespace proptest
