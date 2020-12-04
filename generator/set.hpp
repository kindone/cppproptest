#pragma once
#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "../shrinker/set.hpp"
#include "util.hpp"
#include "../util/std.hpp"

namespace std {

template <typename T>
class less<proptest::Shrinkable<T>> {
public:
    constexpr bool operator()(const proptest::Shrinkable<T>& lhs, const proptest::Shrinkable<T>& rhs) const
    {
        return lhs.getRef() < rhs.getRef();
    }
};

}  // namespace std

namespace proptest {

template <typename T>
class Arbi<set<T>> final : public ArbiContainer<set<T>> {
    using Set = set<T>;
    using ArbiContainer<Set>::minSize;
    using ArbiContainer<Set>::maxSize;

public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi() : ArbiContainer<Set>(defaultMinSize, defaultMaxSize), elemGen(Arbi<T>()) {}

    Arbi(const Arbi<T>& _elemGen)
        : ArbiContainer<Set>(defaultMinSize, defaultMaxSize),
          elemGen([_elemGen](Random& rand) -> Shrinkable<T> { return _elemGen(rand); })
    {
    }

    Arbi(GenFunction<T> _elemGen) : ArbiContainer<Set>(defaultMinSize, defaultMaxSize), elemGen(_elemGen) {}

    Shrinkable<Set> operator()(Random& rand) override
    {
        // generate random Ts using elemGen
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        shared_ptr<set<Shrinkable<T>>> shrinkableSet = make_shared<set<Shrinkable<T>>>();

        while (shrinkableSet->size() < size) {
            auto elem = elemGen(rand);
            shrinkableSet->insert(elem);
        }
        return shrinkSet(shrinkableSet, minSize);
    }

    GenFunction<T> elemGen;
};

template <typename T>
size_t Arbi<set<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbi<set<T>>::defaultMaxSize = 200;

}  // namespace proptest
