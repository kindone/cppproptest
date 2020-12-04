#pragma once

#include "../util/std.hpp"
#include "../gen.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"

namespace proptest {

template <typename T>
class Arbi<shared_ptr<T>> final : public ArbiBase<shared_ptr<T>> {
public:
    Arbi() : elemGen(Arbi<T>()) {}

    Arbi(const Arbi<T>& _elemGen) : elemGen([_elemGen](Random& rand) -> Shrinkable<T> { return _elemGen(rand); }) {}

    Arbi(GenFunction<T> _elemGen) : elemGen(_elemGen) {}

    Shrinkable<shared_ptr<T>> operator()(Random& rand) override
    {
        auto gen = Arbi<T>();
        Shrinkable<T> shrinkable = gen(rand);
        return shrinkable.template map<shared_ptr<T>>(+[](const T& obj) { return make_shared<T>(obj); });
    }

    GenFunction<T> elemGen;
};

}  // namespace proptest
