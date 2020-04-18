#pragma once

#include <memory>
#include "testing/gen.hpp"
#include "testing/Shrinkable.hpp"
#include "testing/Random.hpp"

namespace PropertyBasedTesting {

template <typename T>
class Arbitrary<std::shared_ptr<T>> : public Gen<std::shared_ptr<T>> {
public:
    Arbitrary() : elemGen(Arbitrary<T>()) {
    }

    Arbitrary(const Arbitrary<T>& _elemGen)
     : elemGen([_elemGen](Random& rand)->Shrinkable<T>{ return _elemGen(rand); })
     {
    }

    Arbitrary(std::function<Shrinkable<T>(Random&)> _elemGen) : elemGen(_elemGen) {
    }

    Shrinkable<std::shared_ptr<T>> operator()(Random& rand) {
        auto gen = Arbitrary<T>();
        Shrinkable<T> shrinkable = gen(rand);
        return shrinkable.template transform<std::shared_ptr<T>>([](const Shrinkable<T>& shr) {
            return make_shrinkable<std::shared_ptr<T>>(shr.getSharedPtr());
        });
    }

    std::function<Shrinkable<T>(Random&)> elemGen;
};

}