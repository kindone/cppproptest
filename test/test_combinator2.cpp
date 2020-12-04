#include "testbase.hpp"

using namespace proptest;

TEST(PropTest, TestTransform)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    Random savedRand = rand;

    Arbi<int> gen;

    {
        auto stringGen = transform<int, string>(
            gen, +[](int& value) { return "(" + to_string(value) + ")"; });

        for (int i = 0; i < 10; i++) {
            auto shrinkable = stringGen(rand);
            cout << "string: " << shrinkable.get() << endl;
            int j = 0;
            for (auto itr = shrinkable.shrinks().iterator(); itr.hasNext() && j < 3; j++) {
                auto shrinkable2 = itr.next();
                cout << "  shrink: " << shrinkable2.get() << endl;
                int k = 0;
                for (auto itr2 = shrinkable2.shrinks().iterator(); itr2.hasNext() && k < 3; k++) {
                    cout << "    shrink: " << itr2.next().get() << endl;
                }
            }
        }

        auto vectorGen = transform<string, vector<string>>(
            stringGen, +[](string& value) {
                vector<string> vec;
                vec.push_back(value);
                return vec;
            });

        for (int i = 0; i < 10; i++) {
            cout << "vector " << vectorGen(rand).get()[0] << endl;
        }
    }

    {
        auto stringGen = Arbi<int>().map<string>(+[](int& value) { return "(" + to_string(value) + ")"; });

        for (int i = 0; i < 10; i++) {
            auto shrinkable = stringGen(savedRand);
            cout << "string2: " << shrinkable.get() << endl;
            int j = 0;
            for (auto itr = shrinkable.shrinks().iterator(); itr.hasNext() && j < 3; j++) {
                auto shrinkable2 = itr.next();
                cout << "  shrink2: " << shrinkable2.get() << endl;
                int k = 0;
                for (auto itr2 = shrinkable2.shrinks().iterator(); itr2.hasNext() && k < 3; k++) {
                    cout << "    shrink2: " << itr2.next().get() << endl;
                }
            }
        }

        auto vectorGen = stringGen.map<vector<string>>(+[](string& value) {
            vector<string> vec;
            vec.push_back(value);
            return vec;
        });

        for (int i = 0; i < 10; i++) {
            cout << "vector2 " << vectorGen(savedRand).get()[0] << endl;
        }
    }
}

TEST(PropTest, TestTranform2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    static auto gen = transform<uint8_t, uint8_t>(
        Arbi<uint8_t>(), +[](uint8_t& vbit) { return (1 << 0) & vbit; });

    for (int i = 0; i < 10; i++)
        cout << gen(rand).get() << endl;
}

TEST(PropTest, TestDependency)
{
    auto intGen = interval(0, 2);
    auto pairGen = dependency<int, vector<int>>(
        intGen, +[](int& in) {
            auto intGen = interval<int>(0, 8);
            auto vecGen = Arbi<vector<int>>(intGen);
            vecGen.maxSize = in;
            vecGen.minSize = in;
            return vecGen;
        });

    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++) {
        auto pair = pairGen(rand).get();
        cout << "(" << pair.first << ", " << pair.second << ")" << endl;
    }
    cout << "exhaustive: " << endl;

    for (int i = 0; i < 3; i++) {
        auto pairShr = pairGen(rand);
        exhaustive(pairShr, 0);
    }
}

TEST(PropTest, TestDependency2)
{
    using Dimension = pair<int, uint16_t>;
    using IndexVector = vector<pair<uint16_t, bool>>;
    using RawData = pair<Dimension, IndexVector>;
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto numRowsGen = interval<int>(10000, 10000);
    auto numElementsGen = Arbi<uint16_t>();
    auto dimGen = pairOf(numRowsGen, numElementsGen);

    auto rawGen = dependency<Dimension, IndexVector>(
        dimGen, +[](Dimension& dimension) {
            int numRows = dimension.first;
            uint16_t numElements = dimension.second;
            auto firstGen = interval<uint16_t>(0, numElements);
            auto secondGen = Arbi<bool>();  // TODO true : false should be 2:1
            auto indexGen = pairOf(firstGen, secondGen);
            auto indexVecGen = Arbi<IndexVector>(indexGen);
            indexVecGen.setSize(numRows);
            return indexVecGen;
        });

    cout << "raw." << endl;
    double t0 = getTime();
    for (int i = 0; i < 10; i++) {
        // cout << "rawGen: " << rawGen(rand).get() << " / raw " << i << endl;
        rawGen(rand).get();
        double time = getTime();
        cout << "rawGen: " << i << " time: " << time - t0 << endl;
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
    cout << "transformed." << endl;

    for (int i = 0; i < 10; i++) {
        cout << "table: " << tableDataGen(rand).get() << " / table " << i << endl;
    }

    auto tableDataWithValueGen = dependency<TableData, vector<bool>>(
        tableDataGen, +[](TableData& td) {
            vector<bool> values;
            auto vectorGen = Arbi<vector<bool>>();
            vectorGen.setSize(td.num_elements);
            return vectorGen;
        });

    // exhaustive(tableDataGen(rand), 0);

    // DictionaryCompression::IQTypeInfo ti;
    forAll(
        +[](pair<TableData, vector<bool>>) {
            // column->set(&index[i].first, index[i].second);
            return true;
        },
        tableDataWithValueGen);
}

TEST(PropTest, TestDependency3)
{
    auto nullableIntegers = dependency<bool, int>(
        Arbi<bool>(), +[](bool& isNull) -> GenFunction<int> {
            if (isNull)
                return just(0);
            else
                return interval<int>(10, 20);
        });

    Arbi<bool>().pairWith<int>(+[](bool& value) {
        if (value)
            return interval(0, 10);
        else
            return interval(10, 20);
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
    auto intStringGen = dependency<int, string>(intGen, [](int& value) {
        auto gen = Arbi<string>();
        gen.setMaxSize(value);
        return gen;
    });

    auto stringGen = intStringGen.map<string>([](pair<int, string>& pair) { return pair.second; });

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

    auto nullableIntegers = Arbi<bool>().tupleWith<int>(+[](bool& isNull) -> GenFunction<int> {
        if (isNull)
            return just(0);
        else
            return interval<int>(10, 20);
    });

    auto tupleGen = nullableIntegers.tupleWith<int>(+[](Chain<bool, int>& chain) {
        bool isNull = get<0>(chain);
        int value = get<1>(chain);
        if (isNull)
            return interval(0, value);
        else
            return interval(-10, value);
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

    auto tuple2Gen = Arbi<bool>().tupleWith<int>(+[](bool& value) {
        if (value)
            return interval(0, 10);
        else
            return interval(10, 20);
    });
    auto tuple3Gen = tuple2Gen.tupleWith<string>(+[](tuple<bool, int>& tup) {
        cout << tup << endl;
        if (get<0>(tup)) {
            auto gen = Arbi<string>(interval<char>('A', 'M'));
            gen.setSize(1, 3);
            return gen;
        } else {
            auto gen = Arbi<string>(interval<char>('N', 'Z'));
            gen.setSize(1, 3);
            return gen;
        }
    });

    auto tuple3Gen2 = tuple2Gen.tupleWith<int>(+[](tuple<bool, int>& tup) {
        if (get<0>(tup)) {
            return interval(10, 20);
        } else {
            return interval(20, 30);
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
    auto stringGen = derive<int, string>(intGen, [](int& value) {
        auto gen = Arbi<string>();
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
    auto stringGen = intGen.flatMap<string>([](int& value) {
        auto gen = Arbi<string>();
        gen.setMaxSize(value);
        return gen;
    });

    for (int i = 0; i < 10; i++) {
        auto shr = stringGen(rand);
        exhaustive(shr, 0);
    }
}
