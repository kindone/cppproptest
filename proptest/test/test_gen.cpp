#include "testbase.hpp"

using namespace proptest;

TEST(PropTest, FilterWithTolerance)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto smallIntGen = interval<int>(0, 100);

    for (int i = 0; i < 4; i++) {
        auto shr = smallIntGen(rand);
        auto criteria = [](const int& v) { return v % 4 == 1; };
        if (!criteria(shr.getRef()))
            continue;
        cout << "filter with tolerance:" << endl;
        exhaustive(shr.filter(criteria, 1), 0);
        cout << "filter without tolerance:" << endl;
        exhaustive(shr.filter(criteria), 0);
        cout << "full" << endl;
        exhaustive(shr, 0);
    }
}

TEST(PropTest, GenVectorOfInt)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto smallIntGen = interval<int>(0, 4);
    Arbi<vector<int>> gen(smallIntGen);
    gen.setSize(3);

    // for(int i = 0; i < 20; i++) {
    //     vector<int> val(gen(rand).get());
    //     cout << "vec: ";
    //     for(size_t j = 0; j < val.size(); j++)
    //     {
    //         cout << val[j] << ", ";
    //     }
    //     cout << endl;
    // }
    for (int i = 0; i < 1; i++)
        exhaustive(gen(rand), 0);
}

TEST(PropTest, GenVectorWithNoArbitrary)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto fooGen = construct<Foo, int>(interval<int>(0, 4));
    Arbi<vector<Foo>> gen(fooGen);
    gen.setSize(3);

    for (int i = 0; i < 1; i++)
        exhaustive(gen(rand), 0);
}

TEST(PropTest, ShrinkableAndThen)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto intGen = interval<int>(0, 16);
    auto evenGen = filter<int>(intGen, [](int& val) -> bool { return val % 2 == 0; });

    auto evenShrinkable = evenGen(rand);
    cout << "evenShrinkable: " << evenShrinkable.get() << endl;
    {
        exhaustive(evenShrinkable, 0);
    }

    auto andThen = evenShrinkable.andThenStatic(
        [evenShrinkable]() { return Stream::one(make_shrinkable_any<int>(1000)); });

    cout << "even.andThenStatic([1000]): " << andThen.get() << endl;
    {
        exhaustive(andThen, 0);
    }

    auto andThen2 = evenShrinkable.andThen([evenShrinkable](const Shrinkable<int>& parent) {
        return Stream::one(make_shrinkable_any<int>(parent.get() / 2));
    });

    cout << "even.andThen([n/2]): " << andThen2.get() << endl;
    {
        exhaustive(andThen2, 0);
    }

    auto concat = evenShrinkable.concatStatic(
        [evenShrinkable]() { return Stream::one(make_shrinkable_any<int>(1000)); });

    cout << "even.concatStatic(1000): " << concat.get() << endl;
    {
        exhaustive(concat, 0);
    }

    auto concat2 = evenShrinkable.concat([evenShrinkable](const Shrinkable<int>& parent) {
        return Stream::one(make_shrinkable_any<int>(parent.get() / 2));
    });

    cout << "even.concat(n/2): " << concat2.get() << endl;
    {
        exhaustive(concat2, 0);
    }
}

TEST(PropTest, FloatShrinkable)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto floatGen = Arbi<float>();
    auto shrinkable = floatGen(rand);
    cout << "float generated: " << endl;
    exhaustive(shrinkable, 0);
}

TEST(PropTest, ShrinkableBinary)
{
    using namespace util;
    {
        auto shrinkable = binarySearchShrinkable(0);
        cout << "# binary of 0" << endl;
        exhaustive(shrinkable, 0);
    }
    {
        auto shrinkable = binarySearchShrinkable(1);
        cout << "# binary of 1" << endl;
        exhaustive(shrinkable, 0);
    }
    {
        auto shrinkable = binarySearchShrinkable(8);
        cout << "# binary of 8" << endl;
        exhaustive(shrinkable, 0);
    }

    {
        auto shrinkable = binarySearchShrinkable(7);
        cout << "# binary of 7" << endl;
        exhaustive(shrinkable, 0);
    }

    {
        auto shrinkable = binarySearchShrinkable(9);
        cout << "# binary of 9" << endl;
        exhaustive(shrinkable, 0);
    }

    {
        auto shrinkable = binarySearchShrinkable(-1);
        cout << "# binary of -1" << endl;
        exhaustive(shrinkable, 0);
    }
    {
        auto shrinkable = binarySearchShrinkable(-3);
        cout << "# binary of -3" << endl;
        exhaustive(shrinkable, 0);
    }
    {
        auto shrinkable = binarySearchShrinkable(-8);
        cout << "# binary of -8" << endl;
        exhaustive(shrinkable, 0);
    }

    {
        auto shrinkable = binarySearchShrinkable(-7);
        cout << "# binary of -7" << endl;
        exhaustive(shrinkable, 0);
    }

    {
        auto shrinkable = binarySearchShrinkable(-9);
        cout << "# binary of -9" << endl;
        exhaustive(shrinkable, 0);
    }
}

TEST(PropTest, ShrinkableConcat)
{
    auto shrinkable = util::binarySearchShrinkable(8);

    auto concat = shrinkable.concatStatic([shrinkable]() { return shrinkable.shrinks(); });

    exhaustive(concat, 0);
}

TEST(PropTest, ShrinkVector)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    using T = int;
    int len = 8;
    vector<T> vec;
    vec.reserve(len);
    for (int i = 0; i < len; i++)
        vec.push_back(8);

    // return make_shrinkable<vector<T>>(util::move(vec));

    auto shrinkableVector = util::binarySearchShrinkable(len).map<vector<T>>([vec](const int64_t& len) {
        if (len <= 0)
            return vector<T>();

        auto begin = vec.begin();
        auto last = vec.begin() + len;
        return vector<T>(begin, last);
    });

    auto shrinkableVector2 = shrinkableVector.concat([](const Shrinkable<vector<T>>& shr) {
        vector<T> copy = shr.get();
        if (!copy.empty())
            copy[0] /= 2;
        return Stream(make_shrinkable_any<vector<T>>(copy));
    });

    exhaustive(shrinkableVector, 0);
    exhaustive(shrinkableVector2, 0);
}

TEST(PropTest, ShrinkVectorFromGen)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    using T = int8_t;
    auto genVec = Arbi<vector<T>>(interval<T>(-8, 8));
    genVec.setMaxSize(8);
    genVec.setMinSize(0);
    auto vecShrinkable = genVec(rand);
    // return make_shrinkable<vector<T>>(util::move(vec));
    exhaustive(vecShrinkable, 0);
}

TEST(PropTest, ShrinkSetExhaustive)
{
    static auto combination = [](int n, int r) {
        int result = 1;
        for (int i = 1; i <= r; i++) {
            result *= n--;
            result /= i;
        }
        return result;
    };

    static auto sumCombinations = [](int n, int maxR) {
        if (maxR < 0)
            return 0;
        int result = 0;
        for (int r = 0; r <= maxR; r++)
            result += combination(n, r);
        return result;
    };

    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto minAndMaxSizeGen = interval(0, 10).pairWith([](int& n) { return interval(n, 10); });

    forAll(
        [&rand](pair<int, int> minAndMaxSize) {
            auto elemGen = interval(0, 99);
            int minSize = minAndMaxSize.first;
            int maxSize = minAndMaxSize.second;
            auto setGen = Arbi<set<int>>(elemGen).setSize(minSize, maxSize);
            for (int i = 0; i < 3; i++) {
                auto strSet = set<string>();
                stringstream exhaustiveStr;
                int numTotal = 0;
                auto root = setGen(rand);
                exhaustive<set<int>>(
                    root, 0, [&numTotal, &strSet, &exhaustiveStr](const Shrinkable<set<int>>& shrinkable, int level) {
                        exhaustiveStr << "\n";
                        for (int i = 0; i < level; i++)
                            exhaustiveStr << "  ";
                        exhaustiveStr << proptest::Show<set<int>>(shrinkable.get());
                        numTotal++;

                        stringstream str;
                        str << proptest::Show<set<int>>(shrinkable.get());
                        PROP_EXPECT(strSet.find(str.str()) == strSet.end())
                            << str.str() << " already exists in: " << exhaustiveStr.str();
                        strSet.insert(str.str());
                    });
                auto size = root.getRef().size();
                PROP_EXPECT_EQ(numTotal, pow(2, size) - sumCombinations(size, minSize - 1));
                // cout << "rootSize: "  << size  << ", minSize: " << minSize << ", total: " << numTotal << ", pow: " <<
                // pow(2, size) << ", minus: " << sumCombinations(size, minSize-1) << endl; cout << "exhaustive: " <<
                // exhaustiveStr.str() << endl;
            }
        },
        minAndMaxSizeGen);
}

TEST(PropTest, TuplePair1)
{
    auto intGen = Arbi<int>();
    auto smallIntGen = GenSmallInt();

    auto gen = pairOf(intGen, smallIntGen);
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        cout << gen(rand).get() << endl;
}

TEST(PropTest, TupleGen1)
{
    auto intGen = Arbi<int>();
    auto smallIntGen = GenSmallInt();

    auto gen = tupleOf(intGen, smallIntGen);
    int64_t seed = getCurrentTime();
    Random rand(seed);
    for (int i = 0; i < 10; i++)
        cout << gen(rand).get() << endl;
}

TEST(PropTest, TupleGen2)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    while (true) {
        auto intGen = Arbi<int>();
        auto shrinkable = intGen(rand);
        auto value = shrinkable.get();
        if (value > -20 && value < 20) {
            exhaustive(shrinkable, 0);
            break;
        }
    }

    auto smallIntGen = interval(-40, 40);
    auto tupleGen = tupleOf(smallIntGen, smallIntGen, smallIntGen);
    while (true) {
        auto shrinkable = tupleGen(rand);
        auto valueTup = shrinkable.get();
        auto arg1 = get<0>(valueTup);
        auto arg2 = get<1>(valueTup);
        auto arg3 = get<2>(valueTup);
        if (arg1 > -20 && arg1 < 20 && arg2 > -20 && arg2 < 20 && arg3 > -20 && arg3 < 20) {
            exhaustive(shrinkable, 0);
            break;
        }
    }
}

TEST(PropTest, TupleGen3)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    while (true) {
        auto intGen = Arbi<int>();
        auto shrinkable = intGen(rand);
        auto value = shrinkable.get();
        if (value > -20 && value < 20) {
            break;
        }
    }

    auto smallIntGen = interval(0, 3);
    auto tupleGen = tupleOf(smallIntGen, smallIntGen, smallIntGen);
    for (int i = 0; i < 3; i++) {
        auto shrinkable = tupleGen(rand);
        exhaustive(shrinkable, 0);
    }
}

TEST(PropTest, GenVectorPerf)
{
    struct Log
    {
        Log()
        {
            cout << "construct" << endl;
            breaker();
        }
        Log(const Log&)
        {
            cout << "copy construct" << endl;
            breaker();
        }

        Log(Log&&) { cout << "move construct" << endl; }

        Log& operator=(const Log&)
        {
            cout << "operator=()" << endl;
            return *this;
        }

        void breaker() { cout << "  break()" << endl; }

        ~Log()
        {
            cout << "destruct" << endl;
            breaker();
        }
    };

    int64_t seed = getCurrentTime();
    Random rand(seed);
    auto logGen = Construct<Log>();
    auto vecGen = Arbi<vector<Log>>(logGen);
    vecGen.setSize(1);
    auto shrinkable = vecGen(rand);

    // exhaustive(shrinkable, 0);
}

TEST(PropTest, GenTupleVector)
{
    using IndexVector = vector<tuple<uint16_t, bool>>;
    int64_t seed = getCurrentTime();
    Random rand(seed);

    int numRows = 8;
    uint16_t numElements = 64;
    auto firstGen = interval<uint16_t>(0, numElements);
    auto secondGen = Arbi<bool>();  // TODO true : false should be 2:1
    auto indexGen = tupleOf(firstGen, secondGen);
    auto indexVecGen = Arbi<IndexVector>(indexGen);
    indexVecGen.setMaxSize(numRows);
    indexVecGen.setMinSize(numRows / 2);
    auto shrinkable = indexVecGen(rand);
    exhaustive(shrinkable, 0);
}

TEST(PropTest, GenVectorAndShrink)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    auto smallIntGen = interval<int>(-8, 8);
    auto vectorGen = Arbi<vector<int>>(smallIntGen);
    for (size_t maxLen = 1; maxLen < 4; maxLen++) {
        while (true) {
            vectorGen.setMaxSize(maxLen);
            auto vec = vectorGen(rand);
            if (vec.getRef().size() > (maxLen > 3 ? maxLen - 3 : 0)) {
                exhaustive(vec, 0);
                cout << "printed: " << maxLen << endl;
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

TEST(PropTest, Polymorphic)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);

    struct Vehicle
    {
        virtual ~Vehicle() = default;
        virtual int get() { return 0; }
    };

    struct Car : public Vehicle
    {
        virtual int get() override { return 1; }
    };

    {
        auto carGen = Arbi<int>().map<Vehicle>([](int&) { return Car(); });
        auto carShrinkable = carGen(rand);
        // polymorphism doesn't work!
        cout << "car.get(): " << carShrinkable.getRef().get() << endl;
    }

    {
        auto carGen = lazy<shared_ptr<Vehicle>>([]() { return util::make_shared<Car>(); });
        auto carShrinkable = carGen(rand);
        // polymorphism works
        cout << "car.get(): " << carShrinkable.getRef()->get() << endl;
    }

    {
        auto carGen = Arbi<int>().map<shared_ptr<Vehicle>>([](int&) { return util::make_shared<Car>(); });
        auto carShrinkable = carGen(rand);
        // polymorphism works
        cout << "car.get(): " << carShrinkable.getRef()->get() << endl;
    }
}

struct Constraint
{
    Constraint(int) : id(nextId()) { cout << "Constraint create" << id << endl; }
    Constraint(const Constraint&) = delete;
    Constraint& operator=(const Constraint&) = delete;
    Constraint(Constraint&&)
    {
        id = nextId();
        cout << "Constraint move" << id << endl;
    }
    ~Constraint() { cout << "~Constraint destroy" << id << endl; }

    int id;
    static int maxId;
    static int nextId() { return maxId++; }
};
int Constraint::maxId = 0;

TEST(PropTest, ConstraintObject)
{
    int64_t seed = getCurrentTime();
    Random rand(seed);
    // You cannot directly generate Constraint object, as it's a non-copyable object.
    // But you can create a shared_ptr of Constraint
    auto gen = lazy<shared_ptr<Constraint>>([]() { return util::make_shared<Constraint>(5); });

    gen(rand);
}
