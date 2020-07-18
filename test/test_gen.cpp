#include "testbase.hpp"
#include <iostream>

using namespace proptest;

TEST(PropTest, GenVectorOfInt)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto smallIntGen = fromTo<int>(0, 4);
    Arbitrary<std::vector<int>> gen(smallIntGen);
    gen.setSize(3);

    // for(int i = 0; i < 20; i++) {
    //     std::vector<int> val(gen(rand).get());
    //     std::cout << "vec: ";
    //     for(size_t j = 0; j < val.size(); j++)
    //     {
    //         std::cout << val[j] << ", ";
    //     }
    //     std::cout << std::endl;
    // }
    for (int i = 0; i < 1; i++)
        exhaustive(gen(rand), 0);
}

TEST(PropTest, GenVectorWithNoArbitrary)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto fooGen = construct<Foo, int>(fromTo<int>(0, 4));
    Arbitrary<std::vector<Foo>> gen(fooGen);
    gen.setSize(3);

    for (int i = 0; i < 1; i++)
        exhaustive(gen(rand), 0);
}

TEST(PropTest, ShrinkableAndThen)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto intGen = fromTo<int>(0, 16);
    auto evenGen = filter<int>(intGen, [](const int& val) -> bool { return val % 2 == 0; });

    auto evenShrinkable = evenGen(rand);
    std::cout << "evenShrinkable: " << evenShrinkable.get() << std::endl;
    {
        exhaustive(evenShrinkable, 0);
    }

    auto andThen =
        evenShrinkable.andThen([evenShrinkable]() { return Stream<Shrinkable<int>>::one(make_shrinkable<int>(1000)); });

    std::cout << "even.andThen([1000]): " << andThen.get() << std::endl;
    {
        exhaustive(andThen, 0);
    }

    auto andThen2 = evenShrinkable.andThen([evenShrinkable](const Shrinkable<int>& parent) {
        return Stream<Shrinkable<int>>::one(make_shrinkable<int>(parent.get() / 2));
    });

    std::cout << "even.andThen([n/2]): " << andThen2.get() << std::endl;
    {
        exhaustive(andThen2, 0);
    }

    auto concat =
        evenShrinkable.concat([evenShrinkable]() { return Stream<Shrinkable<int>>::one(make_shrinkable<int>(1000)); });

    std::cout << "even.concat(1000): " << concat.get() << std::endl;
    {
        exhaustive(concat, 0);
    }

    auto concat2 = evenShrinkable.concat([evenShrinkable](const Shrinkable<int>& parent) {
        return Stream<Shrinkable<int>>::one(make_shrinkable<int>(parent.get() / 2));
    });

    std::cout << "even.concat(n/2): " << concat2.get() << std::endl;
    {
        exhaustive(concat2, 0);
    }
}

TEST(PropTest, FloatShrinkable)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto floatGen = Arbitrary<float>();
    auto shrinkable = floatGen(rand);
    std::cout << "float generated: " << std::endl;
    exhaustive(shrinkable, 0);
}

TEST(PropTest, ShrinkableBinary)
{
    using namespace util;
    {
        auto shrinkable = binarySearchShrinkable(0);
        std::cout << "# binary of 0" << std::endl;
        exhaustive(shrinkable, 0);
    }
    {
        auto shrinkable = binarySearchShrinkable(1);
        std::cout << "# binary of 1" << std::endl;
        exhaustive(shrinkable, 0);
    }
    {
        auto shrinkable = binarySearchShrinkable(8);
        std::cout << "# binary of 8" << std::endl;
        exhaustive(shrinkable, 0);
    }

    {
        auto shrinkable = binarySearchShrinkable(7);
        std::cout << "# binary of 7" << std::endl;
        exhaustive(shrinkable, 0);
    }

    {
        auto shrinkable = binarySearchShrinkable(9);
        std::cout << "# binary of 9" << std::endl;
        exhaustive(shrinkable, 0);
    }

    {
        auto shrinkable = binarySearchShrinkable(-1);
        std::cout << "# binary of -1" << std::endl;
        exhaustive(shrinkable, 0);
    }
    {
        auto shrinkable = binarySearchShrinkable(-3);
        std::cout << "# binary of -3" << std::endl;
        exhaustive(shrinkable, 0);
    }
    {
        auto shrinkable = binarySearchShrinkable(-8);
        std::cout << "# binary of -8" << std::endl;
        exhaustive(shrinkable, 0);
    }

    {
        auto shrinkable = binarySearchShrinkable(-7);
        std::cout << "# binary of -7" << std::endl;
        exhaustive(shrinkable, 0);
    }

    {
        auto shrinkable = binarySearchShrinkable(-9);
        std::cout << "# binary of -9" << std::endl;
        exhaustive(shrinkable, 0);
    }
}

TEST(PropTest, ShrinkableConcat)
{
    auto shrinkable = util::binarySearchShrinkable(8);

    auto concat = shrinkable.concat([shrinkable]() { return shrinkable.shrinks(); });

    exhaustive(concat, 0);
}

TEST(PropTest, ShrinkVector)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    using T = int;
    int len = 8;
    std::vector<T> vec;
    vec.reserve(len);
    for (int i = 0; i < len; i++)
        vec.push_back(8);

    // return make_shrinkable<std::vector<T>>(std::move(vec));

    auto shrinkableVector =
        util::binarySearchShrinkable(len).template transform<std::vector<T>>([vec](const int64_t& len) {
            if (len <= 0)
                return std::vector<T>();

            auto begin = vec.begin();
            auto last = vec.begin() + len;
            return std::vector<T>(begin, last);
        });

    auto shrinkableVector2 = shrinkableVector.concat([](const Shrinkable<std::vector<T>>& shr) {
        std::vector<T> copy = shr.get();
        if (!copy.empty())
            copy[0] /= 2;
        return Stream<Shrinkable<std::vector<T>>>(make_shrinkable<std::vector<T>>(copy));
    });

    exhaustive(shrinkableVector, 0);
    exhaustive(shrinkableVector2, 0);
}

TEST(PropTest, ShrinkVectorFromGen)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    using T = int8_t;
    auto genVec = Arbitrary<std::vector<T>>(fromTo<T>(-8, 8));
    genVec.setMaxSize(8);
    genVec.setMinSize(0);
    auto vecShrinkable = genVec(rand);
    // return make_shrinkable<std::vector<T>>(std::move(vec));
    exhaustive(vecShrinkable, 0);
}
TEST(PropTest, TuplePair1)
{
    auto intGen = Arbitrary<int>();
    auto smallIntGen = GenSmallInt();

    auto gen = pair(intGen, smallIntGen);
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        std::cout << gen(rand).get() << std::endl;
}

TEST(PropTest, TupleGen1)
{
    auto intGen = Arbitrary<int>();
    auto smallIntGen = GenSmallInt();

    auto gen = tuple(intGen, smallIntGen);
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        std::cout << gen(rand).get() << std::endl;
}

TEST(PropTest, TupleGen2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    while (true) {
        auto intGen = Arbitrary<int>();
        auto shrinkable = intGen(rand);
        auto value = shrinkable.get();
        if (value > -20 && value < 20) {
            exhaustive(shrinkable, 0);
            break;
        }
    }

    auto smallIntGen = fromTo(-40, 40);
    auto tupleGen = tuple(smallIntGen, smallIntGen, smallIntGen);
    while (true) {
        auto shrinkable = tupleGen(rand);
        auto valueTup = shrinkable.get();
        auto arg1 = std::get<0>(valueTup);
        auto arg2 = std::get<1>(valueTup);
        auto arg3 = std::get<2>(valueTup);
        if (arg1 > -20 && arg1 < 20 && arg2 > -20 && arg2 < 20 && arg3 > -20 && arg3 < 20) {
            exhaustive(shrinkable, 0);
            break;
        }
    }
}

TEST(PropTest, GenVectorPerf)
{
    struct Log
    {
        Log()
        {
            std::cout << "construct" << std::endl;
            breaker();
        }
        Log(const Log&)
        {
            std::cout << "copy construct" << std::endl;
            breaker();
        }

        Log(Log&&) { std::cout << "move construct" << std::endl; }

        Log& operator=(const Log&)
        {
            std::cout << "operator=()" << std::endl;
            return *this;
        }

        void breaker() { std::cout << "  break()" << std::endl; }

        ~Log()
        {
            std::cout << "destruct" << std::endl;
            breaker();
        }
    };

    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto logGen = Construct<Log>();
    auto vecGen = Arbitrary<std::vector<Log>>(logGen);
    vecGen.setSize(1);
    auto shrinkable = vecGen(rand);

    // exhaustive(shrinkable, 0);
}

TEST(PropTest, GenTupleVector)
{
    using IndexVector = std::vector<std::tuple<uint16_t, bool>>;
    int64_t seed = getCurrentTime();
    Random rand(seed);

    int numRows = 8;
    uint16_t numElements = 64;
    auto firstGen = fromTo<uint16_t>(0, numElements);
    auto secondGen = Arbitrary<bool>();  // TODO true : false should be 2:1
    auto indexGen = tuple(firstGen, secondGen);
    auto indexVecGen = Arbitrary<IndexVector>(indexGen);
    indexVecGen.setMaxSize(numRows);
    indexVecGen.setMinSize(numRows / 2);
    auto shrinkable = indexVecGen(rand);
    exhaustive(shrinkable, 0);
}

TEST(PropTest, GenVectorAndShrink)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto smallIntGen = fromTo<int>(-8, 8);
    auto vectorGen = Arbitrary<std::vector<int>>(smallIntGen);
    for (size_t maxLen = 1; maxLen < 4; maxLen++) {
        while (true) {
            vectorGen.setMaxSize(maxLen);
            auto vec = vectorGen(rand);
            if (vec.getRef().size() > (maxLen > 3 ? maxLen - 3 : 0)) {
                exhaustive(vec, 0);
                std::cout << "printed: " << maxLen << std::endl;
                break;
            }
        }
    }
}

TEST(PropTest, ShrinkComplicated)
{
    class Complicated {
    public:
        int value;
        Complicated(int a) : value(a) {}

        Complicated(const Complicated&) = delete;
        Complicated(Complicated&&) = default;

    private:
        Complicated() {}
    };

    auto complicated = make_shrinkable<Complicated>(5);

    auto shrink = []() { return make_shrinkable<Complicated>(5); };

    shrink();
}
