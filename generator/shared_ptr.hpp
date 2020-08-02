#pragma once

#include <memory>
#include "../gen.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"

namespace proptest {

template <typename T>
class Arbitrary<std::shared_ptr<T>> final : public ArbitraryBase<std::shared_ptr<T>> {
public:
    Arbitrary() : elemGen(Arbitrary<T>()) {}

    Arbitrary(const Arbitrary<T>& _elemGen)
        : elemGen([_elemGen](Random& rand) -> Shrinkable<T> { return _elemGen(rand); })
    {
    }

    Arbitrary(GenFunction<T> _elemGen) : elemGen(_elemGen) {}

    Shrinkable<std::shared_ptr<T>> operator()(Random& rand) override
    {
        auto gen = Arbitrary<T>();
        Shrinkable<T> shrinkable = gen(rand);
        return shrinkable.template transform<std::shared_ptr<T>>(
            +[](const T& obj) { return std::make_shared<T>(obj); });
    }

    GenFunction<T> elemGen;
};

}  // namespace proptest
