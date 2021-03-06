#include "testbase.hpp"

using namespace proptest;

TEST(PropTest, andThen)
{
    {
        auto shr = make_shrinkable<int>(0);
        auto shr2 = shr.andThen([](const Shrinkable<int>& parent) {
            return Stream<Shrinkable<int>>::one(make_shrinkable<int>(parent.get()));
        });

        exhaustive(shr2, 0);
    }

    {
        auto shr = make_shrinkable<int>(0).with([]() { return Stream<Shrinkable<int>>::one(make_shrinkable<int>(1)); });
        auto shr2 = shr.andThen([](const Shrinkable<int>& parent) {
            return Stream<Shrinkable<int>>::one(make_shrinkable<int>(parent.get()));
        });

        exhaustive(shr2, 0);
    }
}
