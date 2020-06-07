#pragma once

#include <memory>
#include "../gen.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"

namespace PropertyBasedTesting {

template <typename T>
class Arbitrary<std::shared_ptr<T>> final : public ArbitraryBase<std::shared_ptr<T>> {
public:
    Arbitrary() : elemGen(Arbitrary<T>()) {}

    Arbitrary(const Arbitrary<T>& _elemGen)
        : elemGen([_elemGen](Random& rand) -> Shrinkable<T> { return _elemGen(rand); })
    {
    }

    Arbitrary(std::function<Shrinkable<T>(Random&)> _elemGen) : elemGen(_elemGen) {}

    Shrinkable<std::shared_ptr<T>> operator()(Random& rand) override
    {
        auto gen = Arbitrary<T>();
        Shrinkable<T> shrinkable = gen(rand);
        return shrinkable.template transform<std::shared_ptr<T>>(
            [](const Shrinkable<T>& shr) { return make_shrinkable<std::shared_ptr<T>>(shr.getSharedPtr()); });
    }

    std::function<Shrinkable<T>(Random&)> elemGen;
};

}  // namespace PropertyBasedTesting
