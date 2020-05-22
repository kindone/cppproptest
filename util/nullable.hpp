#pragma once

#include <memory>

namespace PropertyBasedTesting {

template <typename T>
struct Nullable
{
    Nullable() {}
    Nullable(std::shared_ptr<T> ptr) : ptr(ptr) {}

    bool isNull() const { return !static_cast<bool>(ptr); }

    std::shared_ptr<T> ptr;
};

}  // namespace PropertyBasedTesting
