#include "testbase.hpp"

using namespace proptest;

TEST(PropTest, CompilePropInts)
{
    forAll([](int a, int b, int c) -> bool {
        if (a % 2 == 0)
            PROP_DISCARD();
        return true;
    });

    forAll([](std::vector<int> a, std::set<int> b) -> bool {
        if (a.size() % 2 == 0 && b.size() % 2 == 0)
            PROP_DISCARD();
        return true;
    });

    forAll([](std::vector<int> a, std::list<int> b) -> bool {
        if (a.size() % 2 == 0 && b.size() % 2 == 0)
            PROP_DISCARD();
        return true;
    });

    // forAll([](std::vector<int> a, std::shared_ptr<int> b) -> bool {
    //     if (a.size() % 2 == 0 && *b % 2 == 0)
    //         PROP_DISCARD();
    //     return true;
    // });
}
