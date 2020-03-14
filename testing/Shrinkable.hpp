#pragma once

#include <iostream>
#include <memory>
#include "testing/Stream.hpp"

namespace PropertyBasedTesting {


template <typename T>
struct Shrinkable {
    using type = T;

    Shrinkable(const Shrinkable& other) : ptr(other.ptr), shrinks(other.shrinks) {
    }

    Shrinkable with(std::function<Stream<Shrinkable<T>>()> _shrinks) {
        shrinks = _shrinks;
        return *this;
    }

    operator T&&() const { return std::move(*ptr); }
    T get() const { return *ptr; }
    T* getPtr() const { return ptr.get(); }
    T& getRef() const { return *ptr.get(); }

private:
    Shrinkable() {
        shrinks = []() {
            return Stream<Shrinkable<T>>::empty();
        };
    }
    Shrinkable(std::shared_ptr<T>&& p) : ptr(p) {
        shrinks = []() {
            return Stream<Shrinkable<T>>::empty();
        };
    }
 
    std::shared_ptr<T> ptr;
public:
    std::function<Stream<Shrinkable<T>>()> shrinks;

    template <typename U, typename ...Args>
    friend Shrinkable<U> make_shrinkable(Args&&... args);
   
};


template <typename T, typename ...Args>
Shrinkable<T> make_shrinkable(Args&&... args) {
    Shrinkable<T> shrinkable(std::make_shared<T>(args...));
    return shrinkable;
}

/*
template <typename T, typename ...Args>
Shrinkable<T> make_shrinkable(Args&&... args, std::function<Stream<Shrinkable<T>>()> shrinks ) {
    Shrinkable<T> shrinkable(shrinks);
    shrinkable.ptr = std::make_shared<T>(args...);
    return shrinkable;
}
*/

}
