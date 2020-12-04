#pragma once

#include "../gen.hpp"
#include "../Random.hpp"
#include "../util/printing.hpp"
#include "../shrinker/listlike.hpp"
#include "util.hpp"
#include "../util/std.hpp"

namespace proptest {

template <typename T>
class PROPTEST_API Arbi<list<T>> final : public ArbiContainer<list<T>> {
public:
    using List = list<T>;
    using ArbiContainer<List>::minSize;
    using ArbiContainer<List>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi() : ArbiContainer<List>(defaultMinSize, defaultMaxSize), elemGen(Arbi<T>()) {}

    Arbi(const Arbi<T>& _elemGen)
        : ArbiContainer<List>(defaultMinSize, defaultMaxSize),
          elemGen([_elemGen](Random& rand) -> Shrinkable<T> { return _elemGen(rand); })
    {
    }

    Arbi(GenFunction<T> _elemGen) : ArbiContainer<List>(defaultMinSize, defaultMaxSize), elemGen(_elemGen) {}

    using vector_t = vector<Shrinkable<T>>;
    using shrinkable_t = Shrinkable<vector_t>;
    using stream_t = Stream<shrinkable_t>;
    using e_stream_t = Stream<Shrinkable<T>>;

    Shrinkable<list<T>> operator()(Random& rand) override
    {
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        shared_ptr<vector_t> shrinkVec = make_shared<vector_t>();
        shrinkVec->reserve(size);
        for (size_t i = 0; i < size; i++)
            shrinkVec->push_back(elemGen(rand));

        return shrinkListLike<list, T>(shrinkVec, minSize);
    }
    // FIXME: turn to shared_ptr
    GenFunction<T> elemGen;
};

template <typename T>
size_t Arbi<list<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbi<list<T>>::defaultMaxSize = 200;

}  // namespace proptest
