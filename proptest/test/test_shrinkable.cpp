#include "testbase.hpp"

using namespace proptest;

TEST(PropTest, andThen)
{
    {
        auto shr = make_shrinkable<int>(0);
        auto shr2 = shr.andThen([](const Shrinkable<int>& parent) {
            return Stream::one(make_shrinkable_any<int>(parent.get()));
        });

        exhaustive(shr2, 0);
    }

    {
        Shrinkable<int> shr = make_shrinkable<int>(0).with([]() { return Stream::one(make_shrinkable_any<int>(1)); });
        Shrinkable<int> shr2 = shr.andThen([](const Shrinkable<int>& parent) {
            return Stream::one(make_shrinkable<int>(parent.get()));
        });

        exhaustive(shr2, 0);
    }
}
