#pragma once

#include "../gen.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../util/nullable.hpp"
#include "../util/std.hpp"

/**
 * @file nullable.hpp
 * @brief Arbitrary for Nullable<T>
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for Nullable<T> with configurable element generator
 */
template <typename T>
class Arbi<Nullable<T>> final : public ArbiBase<Nullable<T>> {
public:
    Arbi() : elemGen(Arbi<T>()) {}

    Arbi(Arbi<T>& _elemGen) : elemGen([_elemGen](Random& rand) mutable -> Shrinkable<T> { return _elemGen(rand); }) {}

    Arbi(GenFunction<T> _elemGen) : elemGen(_elemGen) {}

    Shrinkable<Nullable<T>> operator()(Random& rand) override
    {
        auto gen = Arbi<T>();
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
