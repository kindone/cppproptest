#include "testbase.hpp"
#include "util/utf8string.hpp"
#include "util/utf16string.hpp"
#include "util/cesu8string.hpp"
#include "util/unicode.hpp"

constexpr int32_t GenSmallInt::boundaryValues[13];

double getTime()
{
    struct timeval time;
    if (gettimeofday(&time, nullptr)) {
        //  Handle error
        return 0;
    }
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

ostream& operator<<(ostream& os, const proptest::UTF8String& str)
{
    os << proptest::Show<proptest::UTF8String>(str);
    return os;
}

ostream& operator<<(ostream& os, const proptest::UTF16BEString& str)
{
    os << proptest::Show<proptest::UTF16BEString>(str);
    return os;
}

ostream& operator<<(ostream& os, const proptest::UTF16LEString& str)
{
    os << proptest::Show<proptest::UTF16LEString>(str);
    return os;
}

ostream& operator<<(ostream& os, const proptest::CESU8String& str)
{
    os << proptest::Show<proptest::CESU8String>(str);
    return os;
}

ostream& operator<<(ostream& os, const vector<Foo>& input)
{
    os << proptest::Show<vector<Foo>>(input);
    return os;
}

ostream& operator<<(ostream& os, const vector<int>& input)
{
    os << proptest::Show<vector<int>>(input);
    return os;
}

ostream& operator<<(ostream& os, const vector<int8_t>& input)
{
    os << proptest::Show<vector<int8_t>>(input);
    return os;
}

ostream& operator<<(ostream& os, const set<int>& input)
{
    os << proptest::Show<set<int>>(input);
    return os;
}

ostream& operator<<(ostream& os, const map<int, int>& input)
{
    os << proptest::Show<map<int, int>>(input);
    return os;
}

ostream& operator<<(ostream& os, const list<int>& input)
{
    os << proptest::Show<list<int>>(input);
    return os;
}

ostream& operator<<(ostream& os, const TableData& td)
{
    os << "{ num_rows: " << td.num_rows << ", num_elements: " << td.num_elements << ", indexes: ";
    os << "[ ";
    if (!td.indexes.empty()) {
        auto pair = td.indexes[0];
        os << "{ first: " << pair.first << ", second: " << pair.second << " }";
    }
    for (size_t i = 1; i < td.indexes.size(); i++) {
        auto pair = td.indexes[i];
        os << ", { first: " << pair.first << ", second: " << pair.second << " }";
    }
    os << " ]";
    return os;
}

ostream& operator<<(ostream& os, const vector<tuple<uint16_t, bool>>& indexVec)
{
    os << "[ ";
    if (!indexVec.empty()) {
        auto tup = indexVec[0];
        os << "{ first: " << get<0>(tup) << ", second: " << get<1>(tup) << " }";
    }
    for (size_t i = 0; i < indexVec.size(); i++) {
        auto tup = indexVec[i];
        os << ", { first: " << get<0>(tup) << ", second: " << get<1>(tup) << " }";
    }
    os << " ]";
    return os;
}

ostream& operator<<(ostream& os, const Animal& input)
{
    os << "{ ";
    os << "numFeet: " << input.numFeet;
    os << ", name: " << input.name;
    os << ", measures: " << input.measures;
    os << " }";
    return os;
}
