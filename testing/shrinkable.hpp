#pragma once

#include <iostream>
#include "testing/Stream.hpp"

namespace PropertyBasedTesting {

template <typename T>
struct Shrinkable {
    using type = T;

    Shrinkable(T&& v) : value(std::move(v)) {
        shrinks = []() {
            return Stream<Shrinkable<T>>::empty();
        };
    }

    Shrinkable(const T& v) : value(v) {
        shrinks = []() {
            return Stream<Shrinkable<T>>::empty();
        };
    }

    Shrinkable(T&& v, std::function<Stream<Shrinkable<T>>()> f) : value(std::move(v)), shrinks(std::move(f)) {
    }

    Shrinkable(const T& v, std::function<Stream<Shrinkable<T>>()> f) : value(v), shrinks(std::move(f)) {
    }


    T value;
    std::function<Stream<Shrinkable<T>>()> shrinks;

    operator T() const { return value; }

};

}
