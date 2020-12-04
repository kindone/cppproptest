#pragma once

#include "../gen.hpp"
#include "../Random.hpp"
#include "../util/printing.hpp"
#include "../shrinker/listlike.hpp"
#include "util.hpp"
#include "../util/std.hpp"

namespace proptest {

template <typename T>
class PROPTEST_API Arbi<vector<T>> final : public ArbiContainer<vector<T>> {
public:
    using Vector = vector<T>;
    using ArbiContainer<vector<T>>::minSize;
    using ArbiContainer<vector<T>>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi() : ArbiContainer<vector<T>>(defaultMinSize, defaultMaxSize), elemGen(Arbi<T>()) {}

    Arbi(const Arbi<T>& _elemGen)
        : ArbiContainer<vector<T>>(defaultMinSize, defaultMaxSize),
          elemGen([_elemGen](Random& rand) -> Shrinkable<T> { return _elemGen(rand); })
    {
    }

    Arbi(GenFunction<T> _elemGen) : ArbiContainer<vector<T>>(defaultMinSize, defaultMaxSize), elemGen(_elemGen) {}

    Shrinkable<vector<T>> operator()(Random& rand) override
    {
        using vector_t = vector<Shrinkable<T>>;
        size_t size = rand.getRandomSize(minSize, maxSize + 1);
        shared_ptr<vector_t> shrinkVec = make_shared<vector_t>();
        shrinkVec->reserve(size);
        for (size_t i = 0; i < size; i++)
            shrinkVec->push_back(elemGen(rand));

        auto result = shrinkListLike<vector, T>(shrinkVec, minSize);
        return result;
    }

    // FIXME: turn to shared_ptr
private:
    GenFunction<T> elemGen;
};

template <typename T>
size_t Arbi<vector<T>>::defaultMinSize = 0;
template <typename T>
size_t Arbi<vector<T>>::defaultMaxSize = 200;

}  // namespace proptest
