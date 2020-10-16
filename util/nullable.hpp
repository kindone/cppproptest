#pragma once

#include <memory>

namespace proptest {

template <typename T>
struct Nullable
{
    Nullable() {}
    Nullable(std::shared_ptr<T> _ptr) : ptr(_ptr) {}

    bool isNull() const { return !static_cast<bool>(ptr); }

    std::shared_ptr<T> ptr;
};

}  // namespace proptest
