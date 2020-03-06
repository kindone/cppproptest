#ifndef __PROPTEST_STREAM_HPP__
#define __PROPTEST_STREAM_HPP__

#include "testing/Seq.hpp"

namespace PropertyBasedTesting
{

template <typename T>
struct Stream;

template <typename T>
struct Iterator {
    Iterator(const Stream<T>& str) : stream(str) {
    }

    bool hasNext() const {
        return !stream.isEmpty();
    }

    T next() {
        if(!hasNext())
            throw std::invalid_argument("iterator has no more next");
        T value = stream.head();
        stream = stream.tail();
        return value;
    }

    Stream<T> stream;
};


template <typename T>
struct StreamImpl {
    using type = T;
    virtual bool isEmpty() const = 0;
    virtual T head() const = 0;
    virtual std::shared_ptr<StreamImpl<T>> tail() const = 0;
    virtual Iterator<T> iterator() const = 0;
};


template <typename T>
struct EmptyStream : public StreamImpl<T> {
    using type = T;

    virtual bool isEmpty() const { return true; }
    virtual T head() const {
        throw std::invalid_argument("attempt to take head from empty stream");
    }
    virtual std::shared_ptr<StreamImpl<T>> tail() const {
        return std::make_shared<EmptyStream>();
    }

    virtual Iterator<T> iterator() const {
        return Iterator<T>{Stream<T>(*this)};
    }
};

template <typename T>
struct NonEmptyStream : public StreamImpl<T> {
    using type = T;

    NonEmptyStream(const T& h, const std::function<Stream<T>()>& gen) :  _head(h), tailGen(gen) {
    }

    virtual bool isEmpty() const {
        return false;
    }

    virtual T head() const {
        return _head;
    }

    virtual std::shared_ptr<StreamImpl<T>> tail() const {
        if(isEmpty())
            return std::make_shared<EmptyStream<T>>();

        return tailGen().impl;
    }

    virtual Iterator<T> iterator() const {
        return Iterator<T>{Stream<T>{*this}};
    }

    T _head;
    std::function<Stream<T>()> tailGen;
};



template <typename T>
struct Stream {
    using type = T;
    Stream(const Stream& other) : impl(other.impl) {
    }
    
    Stream(const std::shared_ptr<StreamImpl<T>>& otherImpl) :  impl(otherImpl) {
    }

    Stream(const EmptyStream<T>& otherImpl) :  impl(std::make_shared<EmptyStream<T>>(otherImpl)) {
    }

    Stream(const NonEmptyStream<T>& otherImpl) :  impl(std::make_shared<NonEmptyStream<T>>(otherImpl)) {
    }
    
    Stream(const T& h, std::function<Stream<T>()> gen) :  impl(std::make_shared<NonEmptyStream<T>>(h, gen)) {
    }



    bool isEmpty() const {
        return impl->isEmpty();
    }

    T head() const {
        return impl->head();
    }

    Stream<T> tail() const {
        return impl->tail();
    }

    Iterator<T> iterator() const {
        return impl->iterator();
    }

    std::shared_ptr<StreamImpl<T>> impl;


    static Stream<T> empty() {
        return Stream(std::make_shared<EmptyStream<T>>());
    }

    static Stream<T> one(T&& a)  {
        return Stream(a, []()->Stream<T> {
            return empty();
        });
    }

    static Stream<T> two(T&& a, T&& b)  {
        return Stream(a, [=]()->Stream<T> {
            return Stream(b, []()->Stream<T> {
                return empty();
            });
        });
    }

};




} // namespace

#endif

