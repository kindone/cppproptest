#include "testbase.hpp"

using namespace proptest;

TEST(PropTest, TestJust)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto gen = just(0);

    std::cout << "just: " << gen(rand).get() << std::endl;
}

TEST(PropTest, TestJust2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto ptrGen = just<std::shared_ptr<int>>(std::make_shared<int>(0));
    auto vecGen = Arbitrary<std::vector<std::shared_ptr<int>>>(ptrGen);

    std::cout << "just: " << ptrGen(rand).get() << std::endl;
    auto shr = vecGen(rand);
    exhaustive(shr, 0);
    // std::cout << "vector: " << vecGen(rand).get() << std::endl;
}

TEST(PropTest, TestLazy)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto ptrGen = lazy<std::shared_ptr<int>>([]() { return std::make_shared<int>(1); });
    auto vecGen = Arbitrary<std::vector<std::shared_ptr<int>>>(ptrGen);

    auto vec = ptrGen(rand).get();

    std::cout << "lazy: " << vec << std::endl;
    auto shr = vecGen(rand);
    exhaustive(shr, 0);
    // std::cout << "vector: " << vecGen(rand).get() << std::endl;
}

TEST(PropTest, TestConstruct)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto gen = construct<Animal, int, std::string, std::vector<int>&>();
    Animal animal = gen(rand).get();
    std::cout << "Gen animal: " << animal << std::endl;

    forAll(
        +[](Animal animal) -> bool {
            PROP_STAT(animal.numFeet > 3);
            return true;
        },
        gen);
}

TEST(PropTest, TestConstruct2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto gen = construct<Animal, int, std::string, std::vector<int>&>(fromTo<int>(0, 10), Arbitrary<std::string>(),
                                                                      Arbitrary<std::vector<int>>());
    Animal animal = gen(rand).get();
    std::cout << "Gen animal: " << animal << std::endl;

    forAll(
        +[](Animal animal) -> bool {
            PROP_STAT(animal.numFeet > 3);
            return true;
        },
        gen);
}

TEST(PropTest, TestConstruct3)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto gen = construct<Animal, int, std::string, std::vector<int>&>(fromTo<int>(0, 10));
    Animal animal = gen(rand).get();
    std::cout << "Gen animal: " << animal << std::endl;

    forAll(
        [](Animal animal) -> bool {
            PROP_STAT(animal.numFeet > 3);
            return true;
        },
        gen);
}

TEST(PropTest, TestConstruct4)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto gen = construct<Animal, int, std::string, std::vector<int>&>(fromTo<int>(0, 10), Arbitrary<std::string>());
    Animal animal = gen(rand).get();
    std::cout << "Gen animal: " << animal << std::endl;

    forAll(
        +[](Animal animal) -> bool {
            PROP_STAT(animal.numFeet > 3);
            return true;
        },
        gen);

    auto gen2 = gen.map<int>(+[](Animal& animal) { return animal.numFeet; });
    std::cout << "Gen animal's feet: " << gen2(rand).get() << std::endl;
}

TEST(PropTest, TestFilter)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    Filter<Arbitrary<Animal>> filteredGen(+[](Animal& a) -> bool {
        return a.numFeet >= 0 && a.numFeet < 100 && a.name.size() < 10 && a.measures.size() < 10;
    });

    std::cout << "filtered animal: " << filteredGen(rand).get() << std::endl;
}

TEST(PropTest, TestFilter2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    Arbitrary<int> gen;
    auto evenGen = filter<int>(
        gen, +[](int& value) { return value % 2 == 0; });

    for (int i = 0; i < 10; i++) {
        std::cout << "even: " << evenGen(rand).get() << std::endl;
    }

    auto fours = filter<int>(
        evenGen, +[](int& value) {
            EXPECT_EQ(value % 2, 0);
            return value % 4 == 0;
        });

    for (int i = 0; i < 10; i++) {
        std::cout << "four: " << fours(rand).get() << std::endl;
    }
}

TEST(PropTest, TestFilter3)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Random savedRand = rand;

    Arbitrary<int> intGen;
    {
        auto evenGen = filter<int>(
            intGen, +[](int& val) -> bool { return val % 2 == 0; });

        auto shrinkable = evenGen(rand);
        std::cout << "GenShrinks: " << shrinkable.get() << std::endl;
        auto shrinks = shrinkable.shrinks();
        for (auto itr = shrinks.iterator(); itr.hasNext();) {
            std::cout << "  shrinks: " << itr.next().get() << std::endl;
        }
    }

    {
        auto evenGen = Arbitrary<int>().filter(+[](int& val) -> bool { return val % 2 == 0; });

        auto shrinkable = evenGen(savedRand);
        std::cout << "GenShrinks2: " << shrinkable.get() << std::endl;
        auto shrinks = shrinkable.shrinks();
        for (auto itr = shrinks.iterator(); itr.hasNext();) {
            std::cout << "  shrinks2: " << itr.next().get() << std::endl;
        }
    }
}

TEST(PropTest, TestOneOf)
{
    auto intGen = Arbitrary<int>();
    auto smallIntGen = GenSmallInt();

    auto gen = oneOf<int>(intGen, smallIntGen);
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        std::cout << gen(rand).get() << std::endl;
}

TEST(PropTest, TestOneOfWeighted)
{
    auto intGen = Arbitrary<int>();
    auto smallIntGen = GenSmallInt();

    auto gen = oneOf<int>(just(0), weightedGen<int>(just(1), 0.2));
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        std::cout << gen(rand).get() << std::endl;
}

TEST(PropTest, TestElementOf)
{
    auto gen = elementOf<int>(0, 2);
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        std::cout << gen(rand).get() << std::endl;
}

TEST(PropTest, TestElementOfWeighted)
{
    auto gen = elementOf<int>(0, weightedVal<int>(1, 0.1), weightedVal(2, 0.1));
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        std::cout << gen(rand).get() << std::endl;
}

TEST(PropTest, TestRanges)
{
    auto intGen = intervals({Interval(-1, 0), Interval(0, 1)});
    auto uintGen = uintervals({UInterval(0, 0), UInterval(10, 10)});
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        std::cout << intGen(rand).get() << std::endl;
    for (int i = 0; i < 10; i++)
        std::cout << uintGen(rand).get() << std::endl;
}
