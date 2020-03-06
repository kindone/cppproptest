#pragma once

#include <iostream>
#include "testing/Stream.hpp"

namespace PropertyBasedTesting {

template <typename T>
struct Shrinkable {
    using type = T;
    Shrinkable(T&& v) : value(std::move(v)) {
    }

    Shrinkable(const T& v) : value(v) {
    }

    T value;

    operator T() const { return value; }

    Stream<Shrinkable<T>> shrinks(Shrinkable<T>& target) {
        return Stream<Shrinkable<T>>::empty();
    }

};

}
