#ifndef TESTING_SHRINKABLE_HPP
#define TESTING_SHRINKABLE_HPP

#include <iostream>

template <typename T>
struct Shrinkable {
    using type = T;
    Shrinkable(T&& v) : value(std::move(v)) {
    }
    T value;

    operator T() const { return value; } 
};

#endif
