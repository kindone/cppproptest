#pragma once

#include "Stream.hpp"
#include "util/std.hpp"

namespace proptest {

template <typename T>
struct Shrinkable;
template <typename T, typename... Args>
Shrinkable<T> make_shrinkable(Args&&... args);

struct ShrinkableBase {
    ShrinkableBase(const shared_ptr<void>& p, const shared_ptr<function<Stream()>>& _shrinks) : ptr(p), shrinksPtr(_shrinks)
    {
    }

    shared_ptr<void> ptr;
    shared_ptr<function<Stream()>> shrinksPtr;
};


template <typename T>
struct Shrinkable : public ShrinkableBase
{
    using type = T;

    Shrinkable(shared_ptr<T> p) : ShrinkableBase(static_pointer_cast<void>(p), emptyPtr()) {}
    Shrinkable(const Shrinkable& other) : ShrinkableBase(other.ptr, other.shrinksPtr) {}

    Shrinkable& operator=(const Shrinkable& other)
    {
        ptr = other.ptr;
        shrinksPtr = other.shrinksPtr;
        return *this;
    }

    Shrinkable with(function<Stream()> _shrinks) const
    {
        return Shrinkable(ptr, util::make_shared<decltype(_shrinks)>(_shrinks));
    }

    Shrinkable with(shared_ptr<function<Stream()>> newShrinksPtr) const
    {
        return Shrinkable(ptr, newShrinksPtr);
    }

    // operator T() const { return get(); }
    T get() const { return *static_pointer_cast<T>(ptr); }
    T* getPtr() const { return static_pointer_cast<T>(ptr).get(); }
    T& getRef() const { return *static_pointer_cast<T>(ptr).get(); }
    shared_ptr<T> getSharedPtr() const { return static_pointer_cast<T>(ptr); }

    template <typename U>
    Shrinkable<U> map(function<U(const T&)> transformer) const
    {
        auto transformerPtr = util::make_shared<decltype(transformer)>(transformer);
        return map<U>(transformerPtr);
    }

    template <typename U>
    Shrinkable<U> map(shared_ptr<function<U(const T&)>> transformerPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        auto shrinkable = make_shrinkable<U>((*transformerPtr)(getRef()));
        return shrinkable.with([thisShrinksPtr, transformerPtr]() {
            return (*thisShrinksPtr)().template transform<Shrinkable<T>, Shrinkable<U>>(
                [transformerPtr](const Shrinkable<T>& shr) { return shr.map(transformerPtr); });
        });
    }

    template <typename U>
    Shrinkable<U> flatMap(function<Shrinkable<U>(const T&)> transformer) const
    {
        auto transformerPtr = util::make_shared<function<Shrinkable<U>(const T&)>>(transformer);
        return flatMap<U>(transformerPtr);
    }

    template <typename U>
    Shrinkable<U> flatMap(shared_ptr<function<Shrinkable<U>(const T&)>> transformerPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        auto shrinkable = (*transformerPtr)(getRef());
        return shrinkable.with([thisShrinksPtr, transformerPtr]() {
            return (*thisShrinksPtr)().template transform<Shrinkable<T>, Shrinkable<U>>(
                [transformerPtr](const Shrinkable<T>& shr) { return shr.flatMap(transformerPtr); });
        });
    }

    template <typename U>
    Shrinkable<U> mapShrinkable(function<Shrinkable<U>(const Shrinkable<T>&)> transformer) const
    {
        auto transformerPtr = util::make_shared<function<Shrinkable<U>(const Shrinkable<T>&)>>(transformer);
        return mapShrinkable<U>(transformerPtr);
    }

    template <typename U>
    Shrinkable<U> mapShrinkable(
        shared_ptr<function<Shrinkable<U>(const Shrinkable<T>&)>> transformerPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        auto shrinkable = (*transformerPtr)(*this);
        return shrinkable.with([thisShrinksPtr, transformerPtr]() {
            return (*thisShrinksPtr)().template transform<Shrinkable<T>, Shrinkable<U>>(
                [transformerPtr](const Shrinkable<T>& shr) { return shr.mapShrinkable(transformerPtr); });
        });
    }

    // provide filtered generation, shrinking
    Shrinkable<T> filter(function<bool(const T&)> criteria) const
    {
        auto criteriaPtr = util::make_shared<function<bool(const T&)>>(criteria);
        return filter(criteriaPtr);
    }

    Shrinkable<T> filter(shared_ptr<function<bool(const T&)>> criteriaPtr) const
    {
        if (!(*criteriaPtr)(getRef()))
            throw invalid_argument("cannot apply criteria");

        auto thisShrinksPtr = shrinksPtr;

        return with([thisShrinksPtr, criteriaPtr]() {
            auto criteriaForStream = util::make_shared<function<bool(const Shrinkable<T>&)>>(
                [criteriaPtr](const Shrinkable<T>& shr) -> bool { return (*criteriaPtr)(shr.getRef()); });
            // filter stream's value, and then transform each shrinkable to call filter recursively
            return (*thisShrinksPtr)()
                .filter(criteriaForStream)
                .template transform<Shrinkable<T>, Shrinkable<T>>(
                    [criteriaPtr](const Shrinkable<T>& shr) { return shr.filter(criteriaPtr); });
        });
    }

    // provide filtered generation, shrinking
    Shrinkable<T> filter(function<bool(const T&)> criteria, int tolerance) const
    {
        auto criteriaPtr = util::make_shared<function<bool(const T&)>>(criteria);
        return filter(criteriaPtr, tolerance);
    }

    Shrinkable<T> filter(shared_ptr<function<bool(const T&)>> criteriaPtr, int tolerance) const
    {
        if (!(*criteriaPtr)(getRef()))
            throw invalid_argument("cannot apply criteria");

        auto thisShrinksPtr = shrinksPtr;

        static function<Stream(const Stream&,
                                              shared_ptr<function<bool(const Shrinkable<T>&)>>,
                                              int)> filterStream =
            [](const Stream& stream,
                shared_ptr<function<bool(const Shrinkable<T>&)>> _criteriaPtr,
                int _tolerance) -> Stream {
            if (stream.isEmpty()) {
                return Stream::empty();
            } else {
                for (auto itr = stream.iterator<Shrinkable<T>>(); itr.hasNext();) {
                    const Shrinkable<T>& shr = itr.next();
                    if ((*_criteriaPtr)(shr)) {
                        auto tail = itr.stream;
                        return Stream{shr, [_criteriaPtr, tail, _tolerance]() { return filterStream(tail, _criteriaPtr, _tolerance); }};
                    } else {
                        // extract from shr's children
                        auto tail = itr.stream;
                        return filterStream(shr.shrinks().take(_tolerance).concat(tail), _criteriaPtr, _tolerance);
                    }
                }
                return Stream::empty();
            }
        };

        return with([thisShrinksPtr, criteriaPtr, tolerance]() {
            auto criteriaForStream = util::make_shared<function<bool(const Shrinkable<T>&)>>(
                [criteriaPtr](const Shrinkable<T>& shr) -> bool { return (*criteriaPtr)(shr.getRef()); });
            // filter stream's value, and then transform each shrinkable to call filter recursively
            return filterStream((*thisShrinksPtr)(), criteriaForStream, tolerance)
                .template transform<Shrinkable<T>, Shrinkable<T>>(
                    [criteriaPtr, tolerance](const Shrinkable<T>& shr) { return shr.filter(criteriaPtr, tolerance); });
        });
    }

    // concat: continues with then after horizontal dead end
    Shrinkable<T> concatStatic(function<Stream()> then) const
    {
        auto thenPtr = util::make_shared<decltype(then)>(then);
        return concatStatic(thenPtr);
    }

    Shrinkable<T> concatStatic(shared_ptr<function<Stream()>> thenPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        return with([thisShrinksPtr, thenPtr]() {
            auto shrinkablesWithThen = (*thisShrinksPtr)().template transform<Shrinkable<T>, Shrinkable<T>>(
                [thenPtr](const Shrinkable<T>& shr) { return shr.concatStatic(thenPtr); });
            return shrinkablesWithThen.concat((*thenPtr)());
        });
    }

    // concat: extend shrinks stream with function taking parent as argument
    Shrinkable<T> concat(function<Stream(const Shrinkable<T>&)> then) const
    {
        auto thenPtr = util::make_shared<decltype(then)>(then);
        return concat(thenPtr);
    }

    Shrinkable<T> concat(shared_ptr<function<Stream(const Shrinkable<T>&)>> thenPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        return with([copy = *this, thisShrinksPtr, thenPtr]() {
            auto shrinkablesWithThen = (*thisShrinksPtr)().template transform<Shrinkable<T>, Shrinkable<T>>(
                [thenPtr](const Shrinkable<T>& shr) { return shr.concat(thenPtr); });
            return shrinkablesWithThen.concat((*thenPtr)(copy));
        });
    }

    // andThen: continues with then after vertical dead end
    Shrinkable<T> andThenStatic(function<Stream()> then) const
    {
        auto thisShrinksPtr = shrinksPtr;
        auto thenPtr = util::make_shared<decltype(then)>(then);
        return andThenStatic(thenPtr);
    }

    Shrinkable<T> andThenStatic(shared_ptr<function<Stream()>> thenPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        if (shrinks().isEmpty()) {
            return with(thenPtr);
        } else {
            return with([thisShrinksPtr, thenPtr]() {
                return (*thisShrinksPtr)().template transform<Shrinkable<T>, Shrinkable<T>>(
                    [thenPtr](const Shrinkable<T>& shr) { return shr.andThenStatic(thenPtr); });
            });
        }
    }

    Shrinkable<T> andThen(function<Stream(const Shrinkable<T>&)> then) const
    {
        auto thenPtr = util::make_shared<decltype(then)>(then);
        return andThen(thenPtr);
    }

    Shrinkable<T> andThen(shared_ptr<function<Stream(const Shrinkable<T>&)>> thenPtr) const
    {
        auto thisShrinksPtr = shrinksPtr;
        auto selfSharedPtr = util::make_shared<Shrinkable<T>>(*this);
        if (shrinksPtr->operator()().isEmpty()) {
            return with([selfSharedPtr, thenPtr]() { return (*thenPtr)(*selfSharedPtr); });
        } else {
            return with([thisShrinksPtr, thenPtr]() {
                return (*thisShrinksPtr)().template transform<Shrinkable<T>, Shrinkable<T>>(
                    [thenPtr](const Shrinkable<T>& shr) { return shr.andThen(thenPtr); });
            });
        }
    }

    Shrinkable<T> take(int n) const
    {
        auto thisShrinksPtr = shrinksPtr;
        return with([thisShrinksPtr, n]() {
            auto shrinks = (*thisShrinksPtr)().take(n);
            return shrinks.template transform<Shrinkable<T>, Shrinkable<T>>([n](const Shrinkable<T>& shr) { return shr.take(n); });
        });
    }

private:
    Shrinkable() { shrinksPtr = emptyPtr(); }

    Shrinkable(shared_ptr<void> p, shared_ptr<function<Stream()>> s) : ShrinkableBase(p, s) { }
    Shrinkable(shared_ptr<T> p, shared_ptr<function<Stream()>> s) : ShrinkableBase(static_pointer_cast<void>(p), s) { }

    shared_ptr<function<Stream()>> emptyPtr()
    {
        static const auto empty =
            util::make_shared<function<Stream()>>(+[]() { return Stream::empty(); });
        return empty;
    }

public:
    Stream shrinks() const { return (*shrinksPtr)(); }

    template <typename U, typename... Args>
    friend Shrinkable<U> make_shrinkable(Args&&... args);
};

template <typename T, typename... Args>
Shrinkable<T> make_shrinkable(Args&&... args)
{
    Shrinkable<T> shrinkable(util::make_shared<T>(args...));
    return shrinkable;
}

}  // namespace proptest
