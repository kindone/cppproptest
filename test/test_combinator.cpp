#include "testbase.hpp"

using namespace PropertyBasedTesting;

TEST(PropTest, TestConstruct)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto gen = construct<Animal, int, std::string, std::vector<int>&>();
    Animal animal = gen(rand).get();
    std::cout << "Gen animal: " << animal << std::endl;

    forAll(
        [](Animal animal) -> bool {
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
        [](Animal animal) -> bool {
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

TEST(PropTest, TestFilter)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    Filter<Arbitrary<Animal>> filteredGen([](Animal& a) -> bool {
        return a.numFeet >= 0 && a.numFeet < 100 && a.name.size() < 10 && a.measures.size() < 10;
    });

    std::cout << "filtered animal: " << filteredGen(rand).get() << std::endl;
}

TEST(PropTest, TestFilter2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    Arbitrary<int> gen;
    auto evenGen = filter<int>(gen, [](const int& value) { return value % 2 == 0; });

    for (int i = 0; i < 10; i++) {
        std::cout << "even: " << evenGen(rand).get() << std::endl;
    }

    auto fours = filter<int>(evenGen, [](const int& value) {
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
    Arbitrary<int> intGen;
    auto evenGen = filter<int>(intGen, [](const int& val) -> bool { return val % 2 == 0; });

    auto shrinkable = evenGen(rand);
    std::cout << "GenShrinks: " << shrinkable.get() << std::endl;
    auto shrinks = shrinkable.shrinks();
    for (auto itr = shrinks.iterator(); itr.hasNext();) {
        std::cout << "  shrinks: " << itr.next().get() << std::endl;
    }
}

TEST(PropTest, TestTransform)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    Arbitrary<int> gen;
    auto stringGen =
        transform<int, std::string>(gen, [](const int& value) { return "(" + std::to_string(value) + ")"; });

    for (int i = 0; i < 10; i++) {
        auto shrinkable = stringGen(rand);
        std::cout << "string: " << shrinkable.get() << std::endl;
        int j = 0;
        for (auto itr = shrinkable.shrinks().iterator(); itr.hasNext() && j < 3; j++) {
            auto shrinkable2 = itr.next();
            std::cout << "  shrink: " << shrinkable2.get() << std::endl;
            int k = 0;
            for (auto itr2 = shrinkable2.shrinks().iterator(); itr2.hasNext() && k < 3; k++) {
                std::cout << "    shrink: " << itr2.next().get() << std::endl;
            }
        }
    }

    auto vectorGen = transform<std::string, std::vector<std::string>>(stringGen, [](const std::string& value) {
        std::vector<std::string> vec;
        vec.push_back(value);
        return vec;
    });

    for (int i = 0; i < 10; i++) {
        std::cout << "vector " << vectorGen(rand).get()[0] << std::endl;
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

    auto gen = oneOf<int>(just<int>([]() { return 0; }), weighted<int>(just<int>([]() { return 1; }), 0.2));
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        std::cout << gen(rand).get() << std::endl;
}

TEST(PropTest, TestDependency)
{
    auto intGen = fromTo(0, 2);
    auto pairGen = dependency<int, std::vector<int>>(intGen, [](const int& in) {
        auto intGen = fromTo<int>(0, 8);
        auto vecGen = Arbitrary<std::vector<int>>(intGen);
        vecGen.maxSize = in;
        vecGen.minSize = in;
        return vecGen;
    });

    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++) {
        auto pair = pairGen(rand).get();
        std::cout << "(" << pair.first << ", " << pair.second << ")" << std::endl;
    }
    std::cout << "exhaustive: " << std::endl;

    for (int i = 0; i < 3; i++) {
        auto pairShr = pairGen(rand);
        exhaustive(pairShr, 0);
    }
}

TEST(PropTest, TestDependency2)
{
    using Dimension = std::tuple<int, uint16_t>;
    using IndexVector = std::vector<std::tuple<uint16_t, bool>>;
    using RawData = std::pair<Dimension, IndexVector>;
    int64_t seed = getCurrentTime();
    Random rand(seed);

    // auto numRowsGen = inRange<int>(1, 100000+1);
    auto numRowsGen = fromTo<int>(10000, 10000);
    auto numElementsGen = Arbitrary<uint16_t>();
    // auto numElementsGen = inRange<uint16_t>(60000, 60000);
    auto dimGen = tuple(numRowsGen, numElementsGen);

    auto rawGen = dependency<Dimension, IndexVector>(dimGen, [](const Dimension& dimension) {
        int numRows = std::get<0>(dimension);
        uint16_t numElements = std::get<1>(dimension);
        auto firstGen = fromTo<uint16_t>(0, numElements);
        auto secondGen = Arbitrary<bool>();  // TODO true : false should be 2:1
        auto indexGen = tuple(firstGen, secondGen);
        auto indexVecGen = Arbitrary<IndexVector>(indexGen);
        indexVecGen.minSize = numRows;
        indexVecGen.maxSize = numRows;
        return indexVecGen;
    });

    std::cout << "raw." << std::endl;
    double t0 = getTime();
    for (int i = 0; i < 10; i++) {
        // std::cout << "rawGen: " << rawGen(rand).get() << " / raw " << i << std::endl;
        rawGen(rand).get();
        double time = getTime();
        std::cout << "rawGen: " << i << " time: " << time - t0 << std::endl;
        t0 = time;
    }

    return;

    auto tableDataGen = transform<RawData, TableData>(rawGen, [](const RawData& raw) {
        TableData tableData;
        auto dimension = raw.first;
        tableData.num_rows = std::get<0>(dimension);
        tableData.num_elements = std::get<1>(dimension);
        tableData.indexes = raw.second;
        return tableData;
    });
    std::cout << "transformed." << std::endl;

    for (int i = 0; i < 10; i++) {
        std::cout << "table: " << tableDataGen(rand).get() << " / table " << i << std::endl;
    }
    // exhaustive(tableDataGen(rand), 0);

    // DictionaryCompression::IQTypeInfo ti;
    forAll(
        [](TableData td) {
            // column->set(&index[i].first, index[i].second);
            return true;
        },
        tableDataGen);
}
