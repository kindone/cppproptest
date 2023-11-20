#pragma once
#include "proptest/gen.hpp"
#include "proptest/Random.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/shrinker/map.hpp"
#include "proptest/generator/set.hpp"
#include "proptest/util/std.hpp"

/**
 * @file map.hpp
 * @brief Arbitrary for map<Key, T>
 */

namespace proptest {
/**
 * @ingroup Generators
 * @brief Arbitrary for map<Key, T> with configurable Key and T generators and min/max sizes
 */
template <typename Key, typename T>
class Arbi<map<Key, T>> final : public ArbiContainer<map<Key, T>> {
    using Map = map<Key, T>;
    using ArbiContainer<Map>::minSize;
    using ArbiContainer<Map>::maxSize;

public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi() : ArbiContainer<Map>(defaultMinSize, defaultMaxSize), keyGen(Arbi<Key>()), elemGen(Arbi<T>()) {}

    Shrinkable<Map> operator()(Random& rand) override
    {
        // generate random Ts using elemGen
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        shared_ptr<set<Shrinkable<Key>>> shrinkSet = util::make_shared<set<Shrinkable<Key>>>();

        while (shrinkSet->size() < size) {
            auto key = keyGen(rand);
            shrinkSet->insert(key);
        }

        shared_ptr<map<Shrinkable<Key>, Shrinkable<T>>> shrinkableMap =
            util::make_shared<map<Shrinkable<Key>, Shrinkable<T>>>();

        for (auto itr = shrinkSet->begin(); itr != shrinkSet->end(); ++itr) {
            auto elem = elemGen(rand);
            shrinkableMap->insert(pair<Shrinkable<Key>, Shrinkable<T>>(*itr, elem));
        }

        return shrinkMap(shrinkableMap, minSize);
    }

    Arbi<Map> setKeyGen(const Arbi<Key>& _keyGen)
    {
        keyGen = _keyGen;
        return *this;
    }

    Arbi<Map> setElemGen(const Arbi<T>& _elemGen)
    {
        elemGen = _elemGen;
        return *this;
    }

    Arbi<Map> setKeyGen(GenFunction<Key> _keyGen)
    {
        keyGen = _keyGen;
        return *this;
    }

    Arbi<Map> setElemGen(GenFunction<T> _elemGen)
    {
        elemGen = _elemGen;
        return *this;
    }

    GenFunction<Key> keyGen;
    GenFunction<T> elemGen;
};

template <typename Key, typename T>
size_t Arbi<map<Key, T>>::defaultMinSize = 0;
template <typename Key, typename T>
size_t Arbi<map<Key, T>>::defaultMaxSize = 200;

}  // namespace proptest
