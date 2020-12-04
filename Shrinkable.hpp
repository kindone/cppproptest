#pragma once

#include "Stream.hpp"
#include "util/std.hpp"

namespace proptest {

template <typename T>
struct Shrinkable;
template <typename T, typename... Args>
Shrinkable<T> make_shrinkable(Args&&... args);

template <typename T>
struct Shrinkable
{
    using type = T;

    Shrinkable(shared_ptr<T> p) : ptr(p) { shrinksPtr = emptyPtr(); }
    Shrinkable(const Shrinkable& other) : ptr(other.ptr), shrinksPtr(other.shrinksPtr) {}

    Shrinkable& operator=(const Shrinkable& other)
    {
        ptr = other.ptr;
        shrinksPtr = other.shrinksPtr;
        return *this;
    }

    Shrinkable with(function<Stream<Shrinkable<T>>()> _shrinks) const
    {
        return Shrinkable(ptr, make_shared<decltype(_shrinks)>(_shrinks));
    }

    Shrinkable with(shared_ptr<function<Stream<Shrinkable<T>>()>> newShrinksPtr) const
    {
        return Shrinkable(ptr, newShrinksPtr);
    }

    // operator T() const { return get(); }
    T get() const { return *ptr; }
    T* getPtr() const { return ptr.get(); }
    T& getRef() const { return *ptr.get(); }
    shared_ptr<T> getSharedPtr() const { return ptr; }

    template <typename U = T>
    Shrinkable<U> map(function<U(const T&)> transformer) const
    {
        auto transformerPtr = make_shared<decltype(transformer)>(transformer);
        return map<U>(transformerPtr);
    }

    template <typename U = T>
    Shrinkable<U> map(shared_ptr<function<U(const T&)>> transformerPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        auto shrinkable = make_shrinkable<U>(move((*transformerPtr)(getRef())));
        return shrinkable.with([thisShrinksPtr, transformerPtr]() {
            return (*thisShrinksPtr)().template transform<Shrinkable<U>>(
                [transformerPtr](const Shrinkable<T>& shr) { return shr.map(transformerPtr); });
        });
    }

    template <typename U = T>
    Shrinkable<U> flatMap(function<Shrinkable<U>(const T&)> transformer) const
    {
        auto transformerPtr = make_shared<function<Shrinkable<U>(const T&)>>(transformer);
        return flatMap<U>(transformerPtr);
    }

    template <typename U = T>
    Shrinkable<U> flatMap(shared_ptr<function<Shrinkable<U>(const T&)>> transformerPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        auto shrinkable = (*transformerPtr)(getRef());
        return shrinkable.with([thisShrinksPtr, transformerPtr]() {
            return (*thisShrinksPtr)().template transform<Shrinkable<U>>(
                [transformerPtr](const Shrinkable<T>& shr) { return shr.flatMap(transformerPtr); });
        });
    }

    template <typename U = T>
    Shrinkable<U> mapShrinkable(function<Shrinkable<U>(const Shrinkable<T>&)> transformer) const
    {
        auto transformerPtr = make_shared<function<Shrinkable<U>(const Shrinkable<T>&)>>(transformer);
        return mapShrinkable<U>(transformerPtr);
    }

    template <typename U = T>
    Shrinkable<U> mapShrinkable(
        shared_ptr<function<Shrinkable<U>(const Shrinkable<T>&)>> transformerPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        auto shrinkable = (*transformerPtr)(*this);
        return shrinkable.with([thisShrinksPtr, transformerPtr]() {
            return (*thisShrinksPtr)().template transform<Shrinkable<U>>(
                [transformerPtr](const Shrinkable<T>& shr) { return shr.mapShrinkable(transformerPtr); });
        });
    }

    // provide filtered generation, shrinking
    Shrinkable<T> filter(function<bool(const T&)> criteria) const
    {
        auto criteriaPtr = make_shared<function<bool(const T&)>>(criteria);
        return filter(criteriaPtr);
    }

    Shrinkable<T> filter(shared_ptr<function<bool(const T&)>> criteriaPtr) const
    {
        if (!(*criteriaPtr)(getRef()))
            throw invalid_argument("cannot apply criteria");

        auto thisShrinksPtr = shrinksPtr;

        return with([thisShrinksPtr, criteriaPtr]() {
            auto criteriaForStream = make_shared<function<bool(const Shrinkable<T>&)>>(
                [criteriaPtr](const Shrinkable<T>& shr) -> bool { return (*criteriaPtr)(shr.getRef()); });
            // filter stream's value, and then transform each shrinkable to call filter recursively
            return (*thisShrinksPtr)()
                .filter(criteriaForStream)
                .template transform<Shrinkable<T>>(
                    [criteriaPtr](const Shrinkable<T>& shr) { return shr.filter(criteriaPtr); });
        });
    }

    // provide filtered generation, shrinking
    Shrinkable<T> filter(function<bool(const T&)> criteria, int tolerance) const
    {
        auto criteriaPtr = make_shared<function<bool(const T&)>>(criteria);
        return filter(criteriaPtr, tolerance);
    }

    Shrinkable<T> filter(shared_ptr<function<bool(const T&)>> criteriaPtr, int tolerance) const
    {
        if (!(*criteriaPtr)(getRef()))
            throw invalid_argument("cannot apply criteria");

        auto thisShrinksPtr = shrinksPtr;

        static function<Stream<Shrinkable<T>>(const Stream<Shrinkable>&,
               shared_ptr<function<bool(const Shrinkable<T>&)>>)> filterStream =
            [tolerance](const Stream<Shrinkable>& stream,
               shared_ptr<function<bool(const Shrinkable<T>&)>> _criteriaPtr) -> Stream<Shrinkable<T>> {
            if (stream.isEmpty()) {
                return Stream<Shrinkable<T>>::empty();
            } else {
                for (auto itr = stream.iterator(); itr.hasNext();) {
                    const Shrinkable<T>& shr = itr.next();
                    if ((*_criteriaPtr)(shr)) {
                        auto tail = itr.stream;
                        return Stream<Shrinkable<T>>{shr, [_criteriaPtr, tail]() { return filterStream(tail, _criteriaPtr); }};
                    } else {
                        // extract from shr's children
                        auto tail = itr.stream;
                        return filterStream(shr.shrinks().take(tolerance).concat(tail), _criteriaPtr);
                    }
                }
                return Stream<Shrinkable<T>>::empty();
            }
        };

        return with([thisShrinksPtr, criteriaPtr, tolerance]() {
            auto criteriaForStream = make_shared<function<bool(const Shrinkable<T>&)>>(
                [criteriaPtr](const Shrinkable<T>& shr) -> bool { return (*criteriaPtr)(shr.getRef()); });
            // filter stream's value, and then transform each shrinkable to call filter recursively
            return filterStream((*thisShrinksPtr)(), criteriaForStream)
                .template transform<Shrinkable<T>>(
                    [criteriaPtr, tolerance](const Shrinkable<T>& shr) { return shr.filter(criteriaPtr, tolerance); });
        });
    }

    // concat: continues with then after horizontal dead end
    Shrinkable<T> concatStatic(function<Stream<Shrinkable<T>>()> then) const
    {
        auto thenPtr = make_shared<decltype(then)>(then);
        return concatStatic(thenPtr);
    }

    Shrinkable<T> concatStatic(shared_ptr<function<Stream<Shrinkable<T>>()>> thenPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        return with([thisShrinksPtr, thenPtr]() {
            auto shrinkablesWithThen = (*thisShrinksPtr)().template transform<Shrinkable<T>>(
                [thenPtr](const Shrinkable<T>& shr) { return shr.concatStatic(thenPtr); });
            return shrinkablesWithThen.concat((*thenPtr)());
        });
    }

    // concat: extend shrinks stream with function taking parent as argument
    Shrinkable<T> concat(function<Stream<Shrinkable<T>>(const Shrinkable<T>&)> then) const
    {
        auto thenPtr = make_shared<decltype(then)>(then);
        return concat(thenPtr);
    }

    Shrinkable<T> concat(shared_ptr<function<Stream<Shrinkable<T>>(const Shrinkable<T>&)>> thenPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        return with([copy = *this, thisShrinksPtr, thenPtr]() {
            auto shrinkablesWithThen = (*thisShrinksPtr)().template transform<Shrinkable<T>>(
                [thenPtr](const Shrinkable<T>& shr) { return shr.concat(thenPtr); });
            return shrinkablesWithThen.concat((*thenPtr)(copy));
        });
    }

    // andThen: continues with then after vertical dead end
    Shrinkable<T> andThenStatic(function<Stream<Shrinkable<T>>()> then) const
    {
        auto thisShrinksPtr = shrinksPtr;
        auto thenPtr = make_shared<decltype(then)>(then);
        return andThenStatic(thenPtr);
    }

    Shrinkable<T> andThenStatic(shared_ptr<function<Stream<Shrinkable<T>>()>> thenPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        if (shrinks().isEmpty()) {
            return with(thenPtr);
        } else {
            return with([thisShrinksPtr, thenPtr]() {
                return (*thisShrinksPtr)().template transform<Shrinkable<T>>(
                    [thenPtr](const Shrinkable<T>& shr) { return shr.andThenStatic(thenPtr); });
            });
        }
    }

    Shrinkable<T> andThen(function<Stream<Shrinkable<T>>(const Shrinkable<T>&)> then) const
    {
        auto thenPtr = make_shared<decltype(then)>(then);
        return andThen(thenPtr);
    }

    Shrinkable<T> andThen(shared_ptr<function<Stream<Shrinkable<T>>(const Shrinkable<T>&)>> thenPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        auto selfSharedPtr = make_shared<Shrinkable<T>>(*this);
        if (shrinksPtr->operator()().isEmpty()) {
            return with([selfSharedPtr, thenPtr]() { return (*thenPtr)(*selfSharedPtr); });
        } else {
            return with([thisShrinksPtr, thenPtr]() {
                return (*thisShrinksPtr)().template transform<Shrinkable<T>>(
                    [thenPtr](const Shrinkable<T>& shr) { return shr.andThen(thenPtr); });
            });
        }
    }

    Shrinkable<T> take(int n) const
    {
        auto thisShrinksPtr = shrinksPtr;
        return with([thisShrinksPtr, n]() {
            auto shrinks = (*thisShrinksPtr)().take(n);
            return shrinks.template transform<Shrinkable<T>>([n](const Shrinkable<T>& shr) { return shr.take(n); });
        });
    }

private:
    Shrinkable() { shrinksPtr = emptyPtr(); }

    Shrinkable(shared_ptr<T> p, shared_ptr<function<Stream<Shrinkable<T>>()>> s) : ptr(p), shrinksPtr(s)
    {
    }

    shared_ptr<function<Stream<Shrinkable<T>>()>> emptyPtr()
    {
        static const auto empty =
            make_shared<function<Stream<Shrinkable<T>>()>>(+[]() { return Stream<Shrinkable<T>>::empty(); });
        return empty;
    }

    shared_ptr<T> ptr;

public:
    Stream<Shrinkable<T>> shrinks() const { return (*shrinksPtr)(); }

    shared_ptr<function<Stream<Shrinkable<T>>()>> shrinksPtr;

    template <typename U, typename... Args>
    friend Shrinkable<U> make_shrinkable(Args&&... args);
};

template <typename T, typename... Args>
Shrinkable<T> make_shrinkable(Args&&... args)
{
    Shrinkable<T> shrinkable(make_shared<T>(args...));
    return shrinkable;
}

}  // namespace proptest
