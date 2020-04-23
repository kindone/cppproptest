#pragma once
#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include <memory>
#include <set>

namespace std {

template<typename T>
class less<PropertyBasedTesting::Shrinkable<T>> {
public:
    constexpr bool operator()(const PropertyBasedTesting::Shrinkable<T> &lhs, const PropertyBasedTesting::Shrinkable<T> &rhs) const
    {
        return lhs.getRef() < rhs.getRef();
    }
};

}


namespace PropertyBasedTesting {

template <typename T>
class Arbitrary<std::set<T>> : public Gen<std::set<T>> {
    using Set = typename std::set<T>;
public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary() : elemGen(Arbitrary<T>()), minSize(defaultMinSize), maxSize(defaultMaxSize) {}

    Arbitrary(const Arbitrary<T>& _elemGen)
        : elemGen([_elemGen](Random& rand) -> Shrinkable<T> { return _elemGen(rand); }),
          minSize(defaultMinSize),
          maxSize(defaultMaxSize)
    {
    }

    Arbitrary(std::function<Shrinkable<T>(Random&)> _elemGen)
        : elemGen(_elemGen), minSize(defaultMinSize), maxSize(defaultMaxSize)
    {
    }

    Shrinkable<Set> operator()(Random& rand) {
        // generate random Ts using elemGen
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        Shrinkable<std::set<Shrinkable<T>>> shrinkables = make_shrinkable<std::set<Shrinkable<T>>>();
        std::set<Shrinkable<T>>& set = shrinkables.getRef();

        while(set.size() < size) {
            auto elem = elemGen(rand);
            set.insert(elem);
        }

        return shrinkables.template transform<std::set<T>>([](const std::set<Shrinkable<T>>& shr) {
            auto value = make_shrinkable<std::set<T>>();
            std::set<T>& valueSet = value.getRef();

            for(auto itr = shr.begin(); itr != shr.end(); ++itr) {
                valueSet.insert(std::move(itr->getRef()));
            }

            return value;
        });
    }

    Arbitrary<std::vector<T>> setMinSize(int size)
    {
        minSize = size;
        return *this;
    }

    Arbitrary<std::vector<T>> setMaxSize(int size)
    {
        maxSize = size;
        return *this;
    }

    Arbitrary<std::vector<T>> setSize(int size)
    {
        minSize = size;
        maxSize = size;
        return *this;
    }

    std::function<Shrinkable<T>(Random&)> elemGen;
    int minSize;
    int maxSize;
};

template <typename T>
size_t Arbitrary<std::set<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbitrary<std::set<T>>::defaultMaxSize = 200;

}
