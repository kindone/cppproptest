#pragma once

#include <functional>
#include <memory>

namespace PropertyBasedTesting {

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
            throw std::invalid_argument("iterator has no more next");
        }
        T value = stream.head();
        stream = stream.tail();
        return value;
    }

    Stream<T> stream;
};

template <typename T>
struct StreamImpl
{
    using type = T;
    virtual ~StreamImpl() {}
    virtual bool isEmpty() const = 0;
    virtual T head() const = 0;
    virtual std::shared_ptr<StreamImpl<T>> tail() const = 0;
    virtual Iterator<T> iterator() const = 0;
};

template <typename T>
struct EmptyStream : public StreamImpl<T>
{
    using type = T;

    virtual ~EmptyStream() {}

    virtual bool isEmpty() const { return true; }
    virtual T head() const { throw std::invalid_argument("attempt to take head from empty stream"); }
    virtual std::shared_ptr<StreamImpl<T>> tail() const { return std::make_shared<EmptyStream>(); }

    virtual Iterator<T> iterator() const { return Iterator<T>{Stream<T>(*this)}; }
};

template <typename T>
struct NonEmptyStream : public StreamImpl<T>
{
    using type = T;

    NonEmptyStream(const T& h, const std::function<Stream<T>()>& gen)
        : _head(h), tailGen(std::make_shared<std::function<Stream<T>()>>(gen))
    {
    }

    virtual ~NonEmptyStream() {}

    virtual bool isEmpty() const { return false; }

    virtual T head() const { return _head; }

    virtual std::shared_ptr<StreamImpl<T>> tail() const
    {
        if (isEmpty())
            return std::make_shared<EmptyStream<T>>();

        return (*tailGen)().impl;
    }

    virtual Iterator<T> iterator() const { return Iterator<T>{Stream<T>{*this}}; }

    template <typename U = T>
    NonEmptyStream<U> transform(std::function<U(const T&)> transformer)
    {
        auto transformerPtr = std::make_shared<decltype(transformer)>(transformer);
        return transform<U>(transformerPtr);
    }

    template <typename U = T>
    NonEmptyStream<U> transform(std::shared_ptr<std::function<U(const T&)>> transformerPtr)
    {
        auto tailGen = this->tailGen;
        return NonEmptyStream<U>((*transformerPtr)(_head), [transformerPtr, tailGen]() -> Stream<U> {
            return (*tailGen)().transform(transformerPtr);
        });
    }

    Stream<T> filter(std::function<bool(const T&)> criteria) const
    {
        auto criteriaPtr = std::make_shared<decltype(criteria)>(criteria);
        return filter(criteriaPtr);
    }

    Stream<T> filter(std::shared_ptr<std::function<bool(const T&)>> criteriaPtr) const
    {
        for (auto itr = iterator(); itr.hasNext();) {
            auto value = itr.next();
            if ((*criteriaPtr)(value)) {
                auto tail = itr.stream;
                return Stream<T>{value, [criteriaPtr, tail]() { return tail.filter(criteriaPtr); }};
            }
        }
        return Stream<T>::empty();
    }

    Stream<T> concat(const Stream<T>& other) const
    {
        return Stream<T>(head(), [tailGen = this->tailGen, other]() { return Stream<T>((*tailGen)()).concat(other); });
    }

    Stream<T> take(int n) const
    {
        auto self = *this;
        if (n == 0)
            return Stream<T>::empty();

        return Stream<T>(head(), [self, n]() { return Stream<T>(self.tail()).take(n - 1); });
    }

    T _head;
    std::shared_ptr<std::function<Stream<T>()>> tailGen;
};

template <typename T>
struct Stream
{
    using type = T;
    Stream(const Stream& other) : impl(other.impl) {}

    Stream(const std::shared_ptr<StreamImpl<T>>& otherImpl) : impl(otherImpl) {}

    Stream(const StreamImpl<T>& otherImpl) : impl(std::make_shared<StreamImpl<T>>(otherImpl)) {}

    Stream(const EmptyStream<T>& otherImpl) : impl(std::make_shared<EmptyStream<T>>(otherImpl)) {}

    Stream(const NonEmptyStream<T>& otherImpl) : impl(std::make_shared<NonEmptyStream<T>>(otherImpl)) {}

    Stream(const T& h, std::function<Stream<T>()> gen) : impl(std::make_shared<NonEmptyStream<T>>(h, gen)) {}

    Stream(const T& h) : impl(std::make_shared<NonEmptyStream<T>>(h, done())) {}

    bool isEmpty() const { return impl->isEmpty(); }

    T head() const { return impl->head(); }

    Stream<T> tail() const { return impl->tail(); }

    Iterator<T> iterator() const { return impl->iterator(); }

    template <typename U>
    Stream<U> transform(std::function<U(const T&)> transformer) const
    {
        auto transformerPtr = std::make_shared<decltype(transformer)>(transformer);
        return transform<U>(transformerPtr);
    }

    template <typename U>
    Stream<U> transform(std::shared_ptr<std::function<U(const T&)>> transformerPtr) const
    {
        if (isEmpty()) {
            return Stream<U>::empty();
        } else {
            return Stream<U>(std::dynamic_pointer_cast<NonEmptyStream<T>>(impl)->transform(transformerPtr));
        }
    }

    Stream<T> filter(std::function<bool(const T&)> criteria) const
    {
        auto criteriaPtr = std::make_shared<decltype(criteria)>(criteria);
        return filter(criteriaPtr);
    }

    Stream<T> filter(std::shared_ptr<std::function<bool(const T&)>> criteriaPtr) const
    {
        if (isEmpty()) {
            return Stream::empty();
        } else {
            return std::dynamic_pointer_cast<NonEmptyStream<T>>(impl)->filter(criteriaPtr);
        }
    }

    Stream<T> concat(const Stream<T>& other) const
    {
        if (isEmpty())
            return other;
        else {
            return std::dynamic_pointer_cast<NonEmptyStream<T>>(impl)->concat(other);
        }
    }

    Stream<T> take(int n) const
    {
        if (isEmpty())
            return empty();
        else
            return std::dynamic_pointer_cast<NonEmptyStream<T>>(impl)->take(n);
    }

    std::shared_ptr<StreamImpl<T>> impl;

    static Stream<T> empty() { return Stream(std::make_shared<EmptyStream<T>>()); }

    static std::function<Stream<T>()> done()
    {
        static auto produceEmpty = []() -> Stream<T> { return empty(); };
        return produceEmpty;
    }

    static Stream<T> one(T&& a) { return Stream(a); }

    static Stream<T> two(T&& a, T&& b)
    {
        return Stream(a, [=]() -> Stream<T> { return Stream(b); });
    }
};

}  // namespace PropertyBasedTesting
