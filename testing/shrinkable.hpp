#pragma once

#include <iostream>
#include <memory>
#include "testing/Stream.hpp"

namespace PropertyBasedTesting {

template <typename T>
struct Shrinkable {
    using type = T;

    Shrinkable(T&& v) : value(std::make_shared<T>(std::move(v))) {
        shrinks = []() {
            return Stream<Shrinkable<T>>::empty();
        };
    }

    Shrinkable(const T& v) : value(std::make_shared<T>(v)) {
        shrinks = []() {
            return Stream<Shrinkable<T>>::empty();
        };
    }

    Shrinkable(T&& v, std::function<Stream<Shrinkable<T>>()> _shrinks) : value(std::make_shared<T>(std::move(v))), shrinks(std::move(_shrinks)) {
    }

    Shrinkable(const T& v, std::function<Stream<Shrinkable<T>>()> _shrinks) : value(std::make_shared<T>(v)), shrinks(std::move(_shrinks)) {
    }


    std::function<Stream<Shrinkable<T>>()> shrinks;

    Shrinkable<T> operator=(const Shrinkable<T>& other) {
        value = other.value;
        shrinks = other.shrinks;
        return *this;
    }

    //operator T&&() const { return std::move(*value); }
    operator T() const { return *value; }

    T get() const { return *value; }
    T move() const { return std::move(*value); }

private:
    std::shared_ptr<T> value;
};

}
