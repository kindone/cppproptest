#pragma once

#include <iostream>
#include <memory>
#include "testing/Stream.hpp"

namespace PropertyBasedTesting {

template <typename T> struct Shrinkable;
template <typename T, typename ...Args>
Shrinkable<T> make_shrinkable(Args&&... args);

template <typename T>
struct Shrinkable {
    using type = T;

    Shrinkable(const Shrinkable& other) : ptr(other.ptr), shrinksPtr(other.shrinksPtr) {
    }

    Shrinkable& operator=(const Shrinkable& other) {
         ptr = other.ptr;
         shrinksPtr = other.shrinksPtr;
         return *this;
    }

    Shrinkable with(std::function<Stream<Shrinkable<T>>()> _shrinks) const {
        auto copy = *this;
        copy.shrinksPtr = std::make_shared<std::function<Stream<Shrinkable<T>>()>>(_shrinks);
        return copy;
    }

    // operator T() const { return get(); }
    T get() const { return *ptr; }
    T* getPtr() const { return ptr.get(); }
    T& getRef() const { return *ptr.get(); }

    template <typename U = T>
    Shrinkable<U> transform(std::function<U(const T&)> transformer) const {
        auto shrinks = this->shrinks();
        auto transformerPtr = std::make_shared<std::function<U(const T&)>>(transformer);
        auto shrinkable = make_shrinkable<U>(std::move(transformer(getRef())));
        return shrinkable.with([shrinks, transformerPtr]() {
            return shrinks.template transform<Shrinkable<U>>([transformerPtr](const Shrinkable<T>& shr) {
                return shr.transform(transformerPtr);
            });
        });
    }

    template <typename U = T>
    Shrinkable<U> transform(std::shared_ptr<std::function<U(const T&)>> transformerPtr) const {
        auto shrinks = this->shrinks();
        auto shrinkable = make_shrinkable<U>(std::move((*transformerPtr)(getRef())));
        return shrinkable.with([shrinks, transformerPtr]() {
            return shrinks.template transform<Shrinkable<U>>([transformerPtr](const Shrinkable<T>& shr) {
                return shr.transform(transformerPtr);
            });
        });
    }

    template <typename U = T>
    Shrinkable<U> transform(std::function<Shrinkable<U>(const T&)> transformer) const {
        auto shrinks = this->shrinks();
        auto transformerPtr = std::make_shared<std::function<Shrinkable<U>(const T&)>>(transformer);
        auto shrinkable = transformer(getRef());
        return shrinkable.with([shrinks, transformerPtr]() {
            return shrinks.template transform<Shrinkable<U>>([transformerPtr](const Shrinkable<T>& shr) {
                return shr.transform(transformerPtr);
            });
        });
    }

    template <typename U = T>
    Shrinkable<U> transform(std::shared_ptr<std::function<Shrinkable<U>(const T&)>> transformerPtr) const {
        auto shrinks = this->shrinks();
        auto shrinkable = (*transformerPtr)(getRef());
        return shrinkable.with([shrinks, transformerPtr]() {
            return shrinks.template transform<Shrinkable<U>>([transformerPtr](const Shrinkable<T>& shr) {
                return shr.transform(transformerPtr);
            });
        });
    }

    // provide filtered generation, shrinking
    Shrinkable<T> filter(std::function<bool(const T&)> criteria) const {
        if(!criteria(getRef()))
            throw std::invalid_argument("cannot apply criteria");

        auto shrinks = this->shrinks();

        return with([shrinks, criteria]() {
            auto criteriaForStream = [criteria](const Shrinkable<T>& shr) -> bool {
                return criteria(shr.getRef());
            };
            // filter stream's value, and then transform each shrinkable to call filter recursively
            return shrinks.filter(criteriaForStream).template transform<Shrinkable<T>>([criteria](const Shrinkable<T>& shr) {
                return shr.filter(criteria);
            });
        });
    }

    // continues with then after horizontal dead end
    Shrinkable<T> concat(std::function<Stream<Shrinkable<T>>()> then) const {
        auto shrinks = this->shrinks();
        return with([shrinks, then]() {
            auto shrinkablesWithThen = shrinks.template transform<Shrinkable<T>>([then](const Shrinkable<T>& shr){
                return shr.concat(then);
            });
            return shrinkablesWithThen.concat(then());
        });
    }

    // extend shrinks stream with function taking parent as argument
    Shrinkable<T> concat(std::function<Stream<Shrinkable<T>>(const Shrinkable<T>&)> then) const {
        auto shrinks = this->shrinks();
        auto copy = *this;
        return with([copy, shrinks, then]() {
            auto shrinkablesWithThen = shrinks.template transform<Shrinkable<T>>([then](const Shrinkable<T>& shr){
                return shr.concat(then);
            });
            return shrinkablesWithThen.concat(then(copy));
        });
    }

    // continues with then after vertical dead end
    Shrinkable<T> andThen(std::function<Stream<Shrinkable<T>>()> then) const {
        auto shrinks = this->shrinks();
        return with([shrinks, then]() {
            return shrinks.template transform<Shrinkable<T>>([then](const Shrinkable<T>& shr){
                if(shr.shrinks().isEmpty())
                    return shr.with(then);
                else
                    return shr.andThen(then);
            });
        });
    }

    Shrinkable<T> andThen(std::function<Stream<Shrinkable<T>>(const Shrinkable<T>&)> then) const {
        auto shrinks = this->shrinks();
        return with([shrinks, then]() {
            return shrinks.template transform<Shrinkable<T>>([then](const Shrinkable<T>& shr){
                if(shr.shrinks().isEmpty())
                    return shr.with([shr, then]() {
                        return then(shr);
                    });
                else
                    return shr.andThen(then);
            });
        });
    }

    Shrinkable<T> take(int n) const {
        auto shrinks = this->shrinks().take(n);
        return with([shrinks, n]() {
            return shrinks.template transform<Shrinkable<T>>([n](const Shrinkable<T>& shr){
                return shr.take(n);
            });
        });
    }

private:
    Shrinkable() {
        shrinksPtr = std::make_shared<std::function<Stream<Shrinkable<T>>()>>([]() {
            return Stream<Shrinkable<T>>::empty();
        });
    }
    Shrinkable(std::shared_ptr<T>&& p) : ptr(p) {
        shrinksPtr = std::make_shared<std::function<Stream<Shrinkable<T>>()>>([]() {
            return Stream<Shrinkable<T>>::empty();
        });
    }

    std::shared_ptr<T> ptr;
public:
    Stream<Shrinkable<T>> shrinks() const {
        return (*shrinksPtr)();
    }

    std::shared_ptr<std::function<Stream<Shrinkable<T>>()>> shrinksPtr;

    template <typename U, typename ...Args>
    friend Shrinkable<U> make_shrinkable(Args&&... args);

};


template <typename T, typename ...Args>
Shrinkable<T> make_shrinkable(Args&&... args) {
    Shrinkable<T> shrinkable(std::make_shared<T>(args...));
    return shrinkable;
}

}
