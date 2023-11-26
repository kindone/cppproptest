#include "proptest/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"



template <typename T>
class ContainerTest : public testing::Test {
public:
    using Type = T;
};

using namespace proptest;

using ContainerTypes = ::testing::Types<vector<uint32_t>, set<uint32_t>, list<uint32_t>,
     Nullable<uint32_t>, shared_ptr<uint32_t>, CESU8String, UTF8String, UTF16BEString, UTF16LEString>;


TYPED_TEST_SUITE(ContainerTest, ContainerTypes);

TYPED_TEST(ContainerTest, various_generators)
{
    Random rand(1);
    //Arbitrary
    auto numGen1 = Arbi<uint32_t>();
    //Generator
    auto numGen2 = inRange<uint32_t>(0,1);
    //GenFunction
    auto numGen3 = [](Random& rand) -> Shrinkable<uint32_t> {
        return make_shrinkable<uint32_t>(0);
    };
    [[maybe_unused]] auto gen1 = Arbi<TypeParam>();
    [[maybe_unused]] auto gen2 = Arbi<TypeParam>(numGen1);
    [[maybe_unused]] auto gen3 = Arbi<TypeParam>(numGen2);
    [[maybe_unused]] auto gen4 = Arbi<TypeParam>(numGen3);
    [[maybe_unused]] auto gen5 = Arbi<TypeParam>(generator(numGen1));
}
