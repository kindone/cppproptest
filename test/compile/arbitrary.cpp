#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"


struct MyObj {
    MyObj(int _a) : a(_a) {}
    int a;
};

struct MyObj2 {
    MyObj2(int _a, int _b) : a(_a), b(_b) {}
    int a;
    int b;
};

using namespace proptest;

DEFINE_ARBITRARY(MyObj, []() {
    auto intGen = interval(10, 20);
    return construct<MyObj, int>(intGen);
});

TEST(Compile, define_arbitrary)
{
    Random rand(1);
    auto myObjGen = Arbi<MyObj>();
    auto shr = myObjGen(rand);
    cout << shr.get().a << endl;
}
