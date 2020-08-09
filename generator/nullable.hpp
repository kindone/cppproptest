#pragma once

#include <memory>
#include "../gen.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../util/nullable.hpp"

namespace proptest {

template <typename T>
class Arbitrary<Nullable<T>> final : public ArbitraryBase<Nullable<T>> {
public:
    Arbitrary() : elemGen(Arbitrary<T>()) {}

    Arbitrary(Arbitrary<T>& _elemGen)
        : elemGen([_elemGen](Random& rand) mutable -> Shrinkable<T> { return _elemGen(rand); })
    {
    }

    Arbitrary(GenFunction<T> _elemGen) : elemGen(_elemGen) {}

    Shrinkable<Nullable<T>> operator()(Random& rand) override
    {
        auto gen = Arbitrary<T>();
        // not null
        if (rand.getRandomBool(0.95)) {
            Shrinkable<T> shrinkable = gen(rand);
            return shrinkable.template mapShrinkable<Nullable<T>>(
                +[](const Shrinkable<T>& shr) { return make_shrinkable<Nullable<T>>(shr.getSharedPtr()); });
        } else {
            // null
            return make_shrinkable<Nullable<T>>();
        }
    }

    GenFunction<T> elemGen;
};

}  // namespace proptest
