
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include "Stream.hpp"
#include "../util/std.hpp"

class StreamTestCase : public ::testing::Test {
};

using namespace proptest;

TEST(StreamTestCase, Stream)
{
    int val = -50;  // 0, -25, ... -49
    // recursive
    auto stream = Stream<int>(0, [val]() {
        static function<Stream<int>(int, int)> genpos = [](int min, int val) {
            if (val <= 0 || (val - min) <= 1)
                return Stream<int>::empty();
            else
                return Stream<int>((val + min) / 2, [min, val]() { return genpos((val + min) / 2, val); });
        };
        static function<Stream<int>(int, int)> genneg = [](int max, int val) {
            if (val >= 0 || (max - val) <= 1)
                return Stream<int>::empty();
            else
                return Stream<int>((val + max) / 2, [max, val]() { return genneg((val + max) / 2, val); });
        };
        if (val >= 0)
            return genpos(0, val);
        else
            return genneg(0, val);
    });

    for (auto itr = stream.iterator(); itr.hasNext();) {
        cout << "stream:" << itr.next() << endl;
    }

    auto emptyIntStream = Stream<int>::empty();
    auto emptyStrStream = Stream<string>::empty();

    auto emptyTransformed =
        emptyIntStream.transform<string>([](const int& value) { return to_string(value); });

    EXPECT_TRUE(emptyIntStream.isEmpty());
    EXPECT_TRUE(emptyTransformed.isEmpty());

    EXPECT_TRUE(emptyIntStream.take(5).isEmpty());

    // empty transform
    auto emptystrstream = emptyIntStream.transform<string>([](const int& value) { return to_string(value); });

    EXPECT_TRUE(emptystrstream.isEmpty());

    auto strstream = stream.transform<string>([](const int& value) { return to_string(value); });

    for (auto itr = strstream.iterator(); itr.hasNext();) {
        cout << "strstream:" << itr.next() << endl;
    }

    auto emptyFiltered = emptyIntStream.filter([](const int& val) { return val == 5; });

    EXPECT_TRUE(emptyFiltered.isEmpty());

    auto filtered = strstream.filter([](const string& str) { return str[1] == '4'; });

    for (auto itr = filtered.iterator(); itr.hasNext();) {
        cout << "filtered:" << itr.next() << endl;
    }

    auto concatenated = strstream.concat(filtered);
    for (auto itr = concatenated.iterator(); itr.hasNext();) {
        cout << "concatenated:" << itr.next() << endl;
    }

    auto take = concatenated.take(4);
    for (auto itr = take.iterator(); itr.hasNext();) {
        cout << "take(4):" << itr.next() << endl;
    }

    auto emptyConcatEmpty = emptyIntStream.concat(Stream<int>::empty());
    EXPECT_TRUE(emptyConcatEmpty.isEmpty());

    auto emptyConcatNonEmpty = emptyStrStream.concat(filtered);
    for (auto itr = emptyConcatNonEmpty.iterator(); itr.hasNext();) {
        cout << "emptyConcatNonEmpty:" << itr.next() << endl;
    }

    auto nonEmptyConcatEmpty = filtered.concat(Stream<string>::empty());
    for (auto itr = nonEmptyConcatEmpty.iterator(); itr.hasNext();) {
        cout << "nonEmptyConcatEmpty:" << itr.next() << endl;
    }
}
