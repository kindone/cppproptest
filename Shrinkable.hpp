#pragma once

#include <iostream>
#include <memory>
#include "Stream.hpp"

namespace PropertyBasedTesting {

template <typename T>
struct Shrinkable;
template <typename T, typename... Args>
Shrinkable<T> make_shrinkable(Args&&... args);

template <typename T>
struct Shrinkable
{
    using type = T;

    Shrinkable(const Shrinkable& other) : ptr(other.ptr), shrinksPtr(other.shrinksPtr) {}

    Shrinkable& operator=(const Shrinkable& other)
    {
        ptr = other.ptr;
        shrinksPtr = other.shrinksPtr;
        return *this;
    }

    Shrinkable with(std::function<Stream<Shrinkable<T>>()> _shrinks) const
    {
        return Shrinkable(this->ptr, std::make_shared<decltype(_shrinks)>(_shrinks));
    }

    Shrinkable with(std::shared_ptr<std::function<Stream<Shrinkable<T>>()>> shrinksPtr) const
    {
        return Shrinkable(this->ptr, shrinksPtr);
    }

    // operator T() const { return get(); }
    T get() const { return *ptr; }
    T* getPtr() const { return ptr.get(); }
    T& getRef() const { return *ptr.get(); }
    std::shared_ptr<T> getSharedPtr() const { return ptr; }

    template <typename U = T>
    Shrinkable<U> transform(std::function<U(const T&)> transformer) const
    {
        auto transformerPtr = std::make_shared<decltype(transformer)>(transformer);
        return transform<U>(transformerPtr);
    }

    template <typename U = T>
    Shrinkable<U> transform(std::shared_ptr<std::function<U(const T&)>> transformerPtr) const
    {
        auto shrinksPtr = this->shrinksPtr;
        auto shrinkable = make_shrinkable<U>(std::move((*transformerPtr)(getRef())));
        return shrinkable.with([shrinksPtr, transformerPtr]() {
            return (*shrinksPtr)().template transform<Shrinkable<U>>(
                [transformerPtr](const Shrinkable<T>& shr) { return shr.transform(transformerPtr); });
        });
    }

    template <typename U = T>
    Shrinkable<U> transform(std::function<Shrinkable<U>(const T&)> transformer) const
    {
        auto transformerPtr = std::make_shared<std::function<Shrinkable<U>(const T&)>>(transformer);
        return transform<U>(transformerPtr);
    }

    template <typename U = T>
    Shrinkable<U> transform(std::shared_ptr<std::function<Shrinkable<U>(const T&)>> transformerPtr) const
    {
        auto shrinksPtr = this->shrinksPtr;
        auto shrinkable = (*transformerPtr)(getRef());
        return shrinkable.with([shrinksPtr, transformerPtr]() {
            return (*shrinksPtr)().template transform<Shrinkable<U>>(
                [transformerPtr](const Shrinkable<T>& shr) { return shr.transform(transformerPtr); });
        });
    }

    template <typename U = T>
    Shrinkable<U> transform(std::function<Shrinkable<U>(const Shrinkable<T>&)> transformer) const
    {
        auto transformerPtr = std::make_shared<std::function<Shrinkable<U>(const Shrinkable<T>&)>>(transformer);
        return transform<U>(transformerPtr);
    }

    template <typename U = T>
    Shrinkable<U> transform(std::shared_ptr<std::function<Shrinkable<U>(const Shrinkable<T>&)>> transformerPtr) const
    {
        auto shrinksPtr = this->shrinksPtr;
        auto shrinkable = (*transformerPtr)(*this);
        return shrinkable.with([shrinksPtr, transformerPtr]() {
            return (*shrinksPtr)().template transform<Shrinkable<U>>(
                [transformerPtr](const Shrinkable<T>& shr) { return shr.transform(transformerPtr); });
        });
    }

    // provide filtered generation, shrinking
    Shrinkable<T> filter(std::function<bool(const T&)> criteria) const
    {
        auto criteriaPtr = std::make_shared<std::function<bool(const T&)>>(criteria);
        return filter(criteriaPtr);
    }

    Shrinkable<T> filter(std::shared_ptr<std::function<bool(const T&)>> criteriaPtr) const
    {
        if (!(*criteriaPtr)(getRef()))
            throw std::invalid_argument("cannot apply criteria");

        auto shrinksPtr = this->shrinksPtr;

        return with([shrinksPtr, criteriaPtr]() {
            auto criteriaForStream = std::make_shared<std::function<bool(const Shrinkable<T>&)>>(
                [criteriaPtr](const Shrinkable<T>& shr) -> bool { return (*criteriaPtr)(shr.getRef()); });
            // filter stream's value, and then transform each shrinkable to call filter recursively
            return (*shrinksPtr)()
                .filter(*criteriaForStream)
                .template transform<Shrinkable<T>>(
                    [criteriaPtr](const Shrinkable<T>& shr) { return shr.filter(criteriaPtr); });
        });
    }

    // concat: continues with then after horizontal dead end
    Shrinkable<T> concat(std::function<Stream<Shrinkable<T>>()> then) const
    {
        auto thenPtr = std::make_shared<decltype(then)>(then);
        return concat(thenPtr);
    }

    Shrinkable<T> concat(std::shared_ptr<std::function<Stream<Shrinkable<T>>()>> thenPtr) const
    {
        auto shrinksPtr = this->shrinksPtr;
        return with([shrinksPtr, thenPtr]() {
            auto shrinkablesWithThen = (*shrinksPtr)().template transform<Shrinkable<T>>(
                [thenPtr](const Shrinkable<T>& shr) { return shr.concat(thenPtr); });
            return shrinkablesWithThen.concat((*thenPtr)());
        });
    }

    // concat: extend shrinks stream with function taking parent as argument
    Shrinkable<T> concat(std::function<Stream<Shrinkable<T>>(const Shrinkable<T>&)> then) const
    {
        auto thenPtr = std::make_shared<decltype(then)>(then);
        return concat(thenPtr);
    }

    Shrinkable<T> concat(std::shared_ptr<std::function<Stream<Shrinkable<T>>(const Shrinkable<T>&)>> thenPtr) const
    {
        auto shrinksPtr = this->shrinksPtr;
        return with([copy = *this, shrinksPtr, thenPtr]() {
            auto shrinkablesWithThen = (*shrinksPtr)().template transform<Shrinkable<T>>(
                [thenPtr](const Shrinkable<T>& shr) { return shr.concat(thenPtr); });
            return shrinkablesWithThen.concat((*thenPtr)(copy));
        });
    }

    // andThen: continues with then after vertical dead end
    Shrinkable<T> andThen(std::function<Stream<Shrinkable<T>>()> then) const
    {
        auto shrinksPtr = this->shrinksPtr;
        auto thenPtr = std::make_shared<decltype(then)>(then);
        return andThen(thenPtr);
    }

    Shrinkable<T> andThen(std::shared_ptr<std::function<Stream<Shrinkable<T>>()>> thenPtr) const
    {
        auto shrinksPtr = this->shrinksPtr;
        return with([shrinksPtr, thenPtr]() {
            return (*shrinksPtr)().template transform<Shrinkable<T>>([thenPtr](const Shrinkable<T>& shr) {
                if (shr.shrinks().isEmpty())
                    return shr.with(thenPtr);
                else
                    return shr.andThen(thenPtr);
            });
        });
    }

    Shrinkable<T> andThen(std::function<Stream<Shrinkable<T>>(const Shrinkable<T>&)> then) const
    {
        auto thenPtr = std::make_shared<decltype(then)>(then);
        return andThen(thenPtr);
    }

    Shrinkable<T> andThen(std::shared_ptr<std::function<Stream<Shrinkable<T>>(const Shrinkable<T>&)>> thenPtr) const
    {
        auto shrinksPtr = this->shrinksPtr;
        return with([shrinksPtr, thenPtr]() {
            return (*shrinksPtr)().template transform<Shrinkable<T>>([thenPtr](const Shrinkable<T>& shr) {
                if (shr.shrinks().isEmpty())
                    return shr.with([shr, thenPtr]() { return (*thenPtr)(shr); });
                else
                    return shr.andThen(thenPtr);
            });
        });
    }

    Shrinkable<T> take(int n) const
    {
        auto shrinksPtr = this->shrinksPtr;
        return with([shrinksPtr, n]() {
            auto shrinks = (*shrinksPtr)().take(n);
            return shrinks.template transform<Shrinkable<T>>([n](const Shrinkable<T>& shr) { return shr.take(n); });
        });
    }

private:
    Shrinkable() { shrinksPtr = emptyPtr(); }
    Shrinkable(std::shared_ptr<T> p) : ptr(p) { shrinksPtr = emptyPtr(); }

    Shrinkable(std::shared_ptr<T> p, std::shared_ptr<std::function<Stream<Shrinkable<T>>()>> s) : ptr(p), shrinksPtr(s)
    {
    }

    std::shared_ptr<std::function<Stream<Shrinkable<T>>()>> emptyPtr()
    {
        static const auto empty =
            std::make_shared<std::function<Stream<Shrinkable<T>>()>>([]() { return Stream<Shrinkable<T>>::empty(); });
        return empty;
    }

    std::shared_ptr<T> ptr;

public:
    Stream<Shrinkable<T>> shrinks() const { return (*shrinksPtr)(); }

    std::shared_ptr<std::function<Stream<Shrinkable<T>>()>> shrinksPtr;

    template <typename U, typename... Args>
    friend Shrinkable<U> make_shrinkable(Args&&... args);
};

template <typename T, typename... Args>
Shrinkable<T> make_shrinkable(Args&&... args)
{
    Shrinkable<T> shrinkable(std::make_shared<T>(args...));
    return shrinkable;
}

}  // namespace PropertyBasedTesting
