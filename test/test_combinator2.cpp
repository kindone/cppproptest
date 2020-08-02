#include "testbase.hpp"

using namespace proptest;

TEST(PropTest, TestTransform)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Random savedRand = rand;

    Arbitrary<int> gen;

    {
        auto stringGen = transform<int, std::string>(
            gen, +[](int& value) { return "(" + std::to_string(value) + ")"; });

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

        auto vectorGen = transform<std::string, std::vector<std::string>>(
            stringGen, +[](std::string& value) {
                std::vector<std::string> vec;
                vec.push_back(value);
                return vec;
            });

        for (int i = 0; i < 10; i++) {
            std::cout << "vector " << vectorGen(rand).get()[0] << std::endl;
        }
    }

    {
        auto stringGen =
            Arbitrary<int>().map<std::string>(+[](int& value) { return "(" + std::to_string(value) + ")"; });

        for (int i = 0; i < 10; i++) {
            auto shrinkable = stringGen(savedRand);
            std::cout << "string2: " << shrinkable.get() << std::endl;
            int j = 0;
            for (auto itr = shrinkable.shrinks().iterator(); itr.hasNext() && j < 3; j++) {
                auto shrinkable2 = itr.next();
                std::cout << "  shrink2: " << shrinkable2.get() << std::endl;
                int k = 0;
                for (auto itr2 = shrinkable2.shrinks().iterator(); itr2.hasNext() && k < 3; k++) {
                    std::cout << "    shrink2: " << itr2.next().get() << std::endl;
                }
            }
        }

        auto vectorGen = stringGen.map<std::vector<std::string>>(+[](std::string& value) {
            std::vector<std::string> vec;
            vec.push_back(value);
            return vec;
        });

        for (int i = 0; i < 10; i++) {
            std::cout << "vector2 " << vectorGen(savedRand).get()[0] << std::endl;
        }
    }
}

TEST(PropTest, TestTranform2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    static auto gen = transform<uint8_t, uint8_t>(
        Arbitrary<uint8_t>(), +[](uint8_t& vbit) { return (1 << 0) & vbit; });

    for (int i = 0; i < 10; i++)
        std::cout << gen(rand).get() << std::endl;
}

TEST(PropTest, TestDependency)
{
    auto intGen = fromTo(0, 2);
    auto pairGen = dependency<int, std::vector<int>>(
        intGen, +[](int& in) {
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
    using Dimension = std::pair<int, uint16_t>;
    using IndexVector = std::vector<std::pair<uint16_t, bool>>;
    using RawData = std::pair<Dimension, IndexVector>;
    int64_t seed = getCurrentTime();
    Random rand(seed);

    // auto numRowsGen = inRange<int>(1, 100000+1);
    auto numRowsGen = fromTo<int>(10000, 10000);
    auto numElementsGen = Arbitrary<uint16_t>();
    // auto numElementsGen = inRange<uint16_t>(60000, 60000);
    auto dimGen = pair(numRowsGen, numElementsGen);

    auto rawGen = dependency<Dimension, IndexVector>(
        dimGen, +[](Dimension& dimension) {
            int numRows = dimension.first;
            uint16_t numElements = dimension.second;
            auto firstGen = fromTo<uint16_t>(0, numElements);
            auto secondGen = Arbitrary<bool>();  // TODO true : false should be 2:1
            auto indexGen = pair(firstGen, secondGen);
            auto indexVecGen = Arbitrary<IndexVector>(indexGen);
            indexVecGen.setSize(numRows);
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

    auto tableDataGen = transform<RawData, TableData>(
        rawGen, +[](RawData& raw) {
            TableData tableData;
            auto dimension = raw.first;
            tableData.num_rows = dimension.first;
            tableData.num_elements = dimension.second;
            tableData.indexes = raw.second;
            return tableData;
        });
    std::cout << "transformed." << std::endl;

    for (int i = 0; i < 10; i++) {
        std::cout << "table: " << tableDataGen(rand).get() << " / table " << i << std::endl;
    }

    auto tableDataWithValueGen = dependency<TableData, std::vector<bool>>(
        tableDataGen, +[](TableData& td) {
            std::vector<bool> values;
            auto vectorGen = Arbitrary<std::vector<bool>>();
            vectorGen.setSize(td.num_elements);
            return vectorGen;
        });

    // exhaustive(tableDataGen(rand), 0);

    // DictionaryCompression::IQTypeInfo ti;
    forAll(
        +[](std::pair<TableData, std::vector<bool>>) {
            // column->set(&index[i].first, index[i].second);
            return true;
        },
        tableDataWithValueGen);
}

TEST(PropTest, TestDependency3)
{
    auto nullableIntegers = dependency<bool, int>(
        Arbitrary<bool>(), +[](bool& isNull) -> GenFunction<int> {
            if (isNull)
                return just(0);
            else
                return fromTo<int>(10, 20);
        });

    Arbitrary<bool>().pair<int>(+[](bool& value) {
        if (value)
            return fromTo(0, 10);
        else
            return fromTo(10, 20);
    });

    int64_t seed = getCurrentTime();
    Random rand(seed);

    for (int i = 0; i < 3; i++) {
        auto pairShr = nullableIntegers(rand);
        exhaustive(pairShr, 0);
    }
}

TEST(PropTest, TestDependency4)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto intGen = elementOf<int>(0, 1, 2, 3);
    auto intStringGen = dependency<int, std::string>(intGen, [](int& value) {
        auto gen = Arbitrary<std::string>();
        gen.setMaxSize(value);
        return gen;
    });

    auto stringGen = intStringGen.map<std::string>([](std::pair<int, std::string>& pair) { return pair.second; });

    Random saveRand = rand;

    for (int i = 0; i < 3; i++) {
        auto shr = intStringGen(rand);
        exhaustive(shr, 0);
    }

    for (int i = 0; i < 3; i++) {
        auto shr = stringGen(saveRand);
        exhaustive(shr, 0);
    }
}

TEST(PropTest, TestChain)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto nullableIntegers = Arbitrary<bool>().tuple<int>(+[](bool& isNull) -> GenFunction<int> {
        if (isNull)
            return just(0);
        else
            return fromTo<int>(10, 20);
    });

    auto tupleGen = nullableIntegers.tuple<int>(+[](Chain<bool, int>& chain) {
        bool isNull = std::get<0>(chain);
        int value = std::get<1>(chain);
        if (isNull)
            return fromTo(0, value);
        else
            return fromTo(-10, value);
    });

    for (int i = 0; i < 3; i++) {
        auto tupleShr = tupleGen(rand);
        exhaustive(tupleShr, 0);
    }
}

TEST(PropTest, TestChain2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto tuple2Gen = Arbitrary<bool>().tuple<int>(+[](bool& value) {
        if (value)
            return fromTo(0, 10);
        else
            return fromTo(10, 20);
    });
    auto tuple3Gen = tuple2Gen.tuple<std::string>(+[](std::tuple<bool, int>& tup) {
        std::cout << tup << std::endl;
        if (std::get<0>(tup)) {
            auto gen = Arbitrary<std::string>(fromTo<char>('A', 'M'));
            gen.setSize(1, 3);
            return gen;
        } else {
            auto gen = Arbitrary<std::string>(fromTo<char>('N', 'Z'));
            gen.setSize(1, 3);
            return gen;
        }
    });

    auto tuple3Gen2 = tuple2Gen.tuple<int>(+[](std::tuple<bool, int>& tup) {
        if (std::get<0>(tup)) {
            return fromTo(10, 20);
        } else {
            return fromTo(20, 30);
        }
    });

    for (int i = 0; i < 3; i++) {
        auto tupleShr = tuple2Gen(rand);
        exhaustive(tupleShr, 0);
    }

    for (int i = 0; i < 3; i++) {
        auto tupleShr = tuple3Gen(rand);
        exhaustive(tupleShr, 0);
    }

    for (int i = 0; i < 3; i++) {
        auto tupleShr = tuple3Gen2(rand);
        exhaustive(tupleShr, 0);
    }
}

TEST(PropTest, TestDerive)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto intGen = elementOf<int>(2, 4, 6);
    auto stringGen = derive<int, std::string>(intGen, [](int& value) {
        auto gen = Arbitrary<std::string>();
        gen.setMaxSize(value);
        return gen;
    });

    for (int i = 0; i < 10; i++) {
        auto shr = stringGen(rand);
        exhaustive(shr, 0);
    }
}

TEST(PropTest, TestDerive2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto intGen = elementOf<int>(2, 4, 6);
    auto stringGen = intGen.template flatMap<std::string>([](int& value) {
        auto gen = Arbitrary<std::string>();
        gen.setMaxSize(value);
        return gen;
    });

    for (int i = 0; i < 10; i++) {
        auto shr = stringGen(rand);
        exhaustive(shr, 0);
    }
}
