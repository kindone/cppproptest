#pragma once

#include "util/std.hpp"

namespace proptest {

template <typename T>
struct Stream;

template <typename T>
struct Iterator
{
    Iterator(const Stream<T>& str) : stream(str) {}

    bool hasNext() const { return !stream.isEmpty(); }

    T next()
    {
        if (!hasNext()) {
            throw invalid_argument("iterator has no more next");
        }
        T value = stream.head();
        stream = stream.tail();
        return value;
    }

    Stream<T> stream;
};

template <typename T>
struct Stream
{
    using type = T;
    Stream() {}
    Stream(const Stream& other) : headPtr(other.headPtr), tailGen(other.tailGen) {}
    Stream(const shared_ptr<Stream<T>>& other) : headPtr(other->headPtr), tailGen(other->tailGen) {}

    Stream(const T& h, function<Stream<T>()> gen)
        : headPtr(util::make_shared<T>(h)), tailGen(util::make_shared<function<Stream<T>()>>(gen))
    {
    }

    Stream(const shared_ptr<T>& h, function<Stream<T>()> gen)
        : headPtr(h), tailGen(util::make_shared<function<Stream<T>()>>(gen))
    {
    }

    Stream(const T& h) : headPtr(util::make_shared<T>(h)), tailGen(util::make_shared<function<Stream<T>()>>(done()))
    {
    }

    bool isEmpty() const { return !static_cast<bool>(headPtr); }

    T head() const { return *headPtr; }

    Stream<T> tail() const
    {
        if (isEmpty())
            return Stream();

        return Stream((*tailGen)());
    }

    Iterator<T> iterator() const { return Iterator<T>{Stream<T>{*this}}; }

    template <typename U = T>
    Stream<U> transform(function<U(const T&)> transformer)
    {
        auto transformerPtr = util::make_shared<decltype(transformer)>(transformer);
        return transform<U>(transformerPtr);
    }

    template <typename U = T>
    Stream<U> transform(shared_ptr<function<U(const T&)>> transformerPtr)
    {
        if (isEmpty()) {
            return Stream<U>::empty();
        } else {
            auto thisTailGen = tailGen;
            return Stream<U>((*transformerPtr)(head()), [transformerPtr, thisTailGen]() -> Stream<U> {
                return (*thisTailGen)().transform(transformerPtr);
            });
        }
    }

    Stream<T> filter(function<bool(const T&)> criteria) const
    {
        auto criteriaPtr = util::make_shared<decltype(criteria)>(criteria);
        return filter(criteriaPtr);
    }

    Stream<T> filter(shared_ptr<function<bool(const T&)>> criteriaPtr) const
    {
        if (isEmpty()) {
            return Stream::empty();
        } else {
            for (auto itr = iterator(); itr.hasNext();) {
                auto value = itr.next();
                if ((*criteriaPtr)(value)) {
                    auto tail = itr.stream;
                    return Stream<T>{value, [criteriaPtr, tail]() { return tail.filter(criteriaPtr); }};
                }
            }
            return Stream<T>::empty();
        }
    }

    Stream<T> concat(const Stream<T>& other) const
    {
        if (isEmpty())
            return other;
        else {
            return Stream<T>(headPtr,
                             [tailGen = this->tailGen, other]() { return Stream((*tailGen)()).concat(other); });
        }
    }

    Stream<T> take(int n) const
    {
        if (isEmpty())
            return empty();
        else {
            auto self = *this;
            if (n == 0)
                return Stream::empty();

            return Stream(headPtr, [self, n]() { return Stream<T>(self.tail()).take(n - 1); });
        }
    }

    shared_ptr<T> headPtr;
    shared_ptr<function<Stream<T>()>> tailGen;

    static Stream<T> empty() { return Stream(); }

    static function<Stream<T>()> done()
    {
        static auto produceEmpty = +[]() -> Stream<T> { return empty(); };
        return produceEmpty;
    }

    static Stream<T> one(T&& a) { return Stream(a); }

    static Stream<T> two(T&& a, T&& b)
    {
        return Stream(a, [=]() -> Stream<T> { return Stream(b); });
    }
};

}  // namespace proptest
