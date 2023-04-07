#include "testbase.hpp"

using namespace proptest;

TEST(PropTest, TestJust)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto gen = just(0);

    cout << "just: " << gen(rand).get() << endl;
}

TEST(PropTest, TestJust2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto ptrGen = just<shared_ptr<int>>(util::make_shared<int>(0));
    auto vecGen = Arbi<vector<shared_ptr<int>>>(ptrGen).setMaxSize(10);

    cout << "just: " << ptrGen(rand).get() << endl;
    auto shr = vecGen(rand);
    exhaustive(shr, 0);
    // cout << "vector: " << vecGen(rand).get() << endl;
}

TEST(PropTest, TestLazy)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto ptrGen = lazy([]() { return util::make_shared<int>(1); });
    auto vecGen = Arbi<vector<shared_ptr<int>>>(ptrGen).setMaxSize(10);

    auto vec = ptrGen(rand).get();

    cout << "lazy: " << vec << endl;
    auto shr = vecGen(rand);
    exhaustive(shr, 0);
    // cout << "vector: " << vecGen(rand).get() << endl;
}

TEST(PropTest, TestConstruct)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto gen = construct<Animal, int, string, vector<int>&>();
    Animal animal = gen(rand).get();
    cout << "Gen animal: " << animal << endl;

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

    auto gen = construct<Animal, int, string, vector<int>&>(interval<int>(0, 10), Arbi<string>(),
                                                                      Arbi<vector<int>>());
    Animal animal = gen(rand).get();
    cout << "Gen animal: " << animal << endl;

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

    auto gen = construct<Animal, int, string, vector<int>&>(interval<int>(0, 10));
    Animal animal = gen(rand).get();
    cout << "Gen animal: " << animal << endl;

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

    auto gen = construct<Animal, int, string, vector<int>&>(interval<int>(0, 10), Arbi<string>());
    Animal animal = gen(rand).get();
    cout << "Gen animal: " << animal << endl;

    forAll(
        +[](Animal animal) -> bool {
            PROP_STAT(animal.numFeet > 3);
            return true;
        },
        gen);

    auto gen2 = gen.map<int>(+[](Animal& animal) { return animal.numFeet; });
    cout << "Gen animal's feet: " << gen2(rand).get() << endl;
}

TEST(PropTest, Tests)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto filteredGen = Arbi<Animal>().filter(+[](Animal& a) -> bool {
        return a.numFeet >= 0 && a.numFeet < 100 && a.name.size() < 10 && a.measures.size() < 10;
    });

    cout << "filtered animal: " << filteredGen(rand).get() << endl;
}

TEST(PropTest, TestFilter2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    Arbi<int> gen;
    auto evenGen = filter<int>(
        gen, +[](int& value) { return value % 2 == 0; });

    for (int i = 0; i < 10; i++) {
        cout << "even: " << evenGen(rand).get() << endl;
    }

    auto fours = filter<int>(
        evenGen, +[](int& value) {
            EXPECT_EQ(value % 2, 0);
            return value % 4 == 0;
        });

    for (int i = 0; i < 10; i++) {
        cout << "four: " << fours(rand).get() << endl;
    }
}

TEST(PropTest, TestFilter3)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Random savedRand = rand;

    Arbi<int> intGen;
    {
        auto evenGen = filter<int>(
            intGen, +[](int& val) -> bool { return val % 2 == 0; });

        auto shrinkable = evenGen(rand);
        cout << "GenShrinks: " << shrinkable.get() << endl;
        auto shrinks = shrinkable.shrinks();
        for (auto itr = shrinks.iterator<Shrinkable<int>>(); itr.hasNext();) {
            cout << "  shrinks: " << itr.next().get() << endl;
        }
    }

    {
        auto evenGen = Arbi<int>().filter(+[](int& val) -> bool { return val % 2 == 0; });

        auto shrinkable = evenGen(savedRand);
        cout << "GenShrinks2: " << shrinkable.get() << endl;
        auto shrinks = shrinkable.shrinks();
        for (auto itr = shrinks.iterator<Shrinkable<int>>(); itr.hasNext();) {
            cout << "  shrinks2: " << itr.next().get() << endl;
        }
    }
}

TEST(PropTest, TestOneOf)
{
    auto intGen = Arbi<int>();
    auto smallIntGen = GenSmallInt();

    auto gen = oneOf<int>(intGen, smallIntGen);
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        cout << gen(rand).get() << endl;
}

TEST(PropTest, TestOneOfWeighted)
{
    auto intGen = Arbi<int>();
    auto smallIntGen = GenSmallInt();

    auto gen = oneOf<int>(just(0), weightedGen(just(1), 0.2));
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        cout << gen(rand).get() << endl;
}

TEST(PropTest, TestElementOf)
{
    auto gen = elementOf<int>(0, 2);
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        cout << gen(rand).get() << endl;
}

TEST(PropTest, TestElementOfWeighted)
{
    auto gen = elementOf<int>(0, weightedVal(1, 0.1), weightedVal(2, 0.1));
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        cout << gen(rand).get() << endl;
}

TEST(PropTest, TestRanges)
{
    auto intGen = intervals({Interval(-1, 0), Interval(0, 1)});
    auto uintGen = uintervals({UInterval(0, 0), UInterval(10, 10)});
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        cout << intGen(rand).get() << endl;
    for (int i = 0; i < 10; i++)
        cout << uintGen(rand).get() << endl;
}
