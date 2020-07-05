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

std::ostream& operator<<(std::ostream& os, const pbt::UTF8String& str)
{
    os << pbt::Show<pbt::UTF8String>(str);
    return os;
}

std::ostream& operator<<(std::ostream& os, const pbt::UTF16BEString& str)
{
    os << pbt::Show<pbt::UTF16BEString>(str);
    return os;
}

std::ostream& operator<<(std::ostream& os, const pbt::UTF16LEString& str)
{
    os << pbt::Show<pbt::UTF16LEString>(str);
    return os;
}

std::ostream& operator<<(std::ostream& os, const pbt::CESU8String& str)
{
    os << pbt::Show<pbt::CESU8String>(str);
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<Foo>& input)
{
    os << pbt::Show<std::vector<Foo>>(input);
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<int>& input)
{
    os << pbt::Show<std::vector<int>>(input);
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<int8_t>& input)
{
    os << pbt::Show<std::vector<int8_t>>(input);
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::set<int>& input)
{
    os << pbt::Show<std::set<int>>(input);
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::map<int, int>& input)
{
    os << pbt::Show<std::map<int, int>>(input);
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::list<int>& input)
{
    os << pbt::Show<std::list<int>>(input);
    return os;
}

std::ostream& operator<<(std::ostream& os, const TableData& td)
{
    os << "{ num_rows: " << td.num_rows << ", num_elements: " << td.num_elements << ", indexes: ";
    os << "[ ";
    if (!td.indexes.empty()) {
        auto tup = td.indexes[0];
        os << "{ first: " << std::get<0>(tup) << ", second: " << std::get<1>(tup) << " }";
    }
    for (size_t i = 1; i < td.indexes.size(); i++) {
        auto tup = td.indexes[i];
        os << ", { first: " << std::get<0>(tup) << ", second: " << std::get<1>(tup) << " }";
    }
    os << " ]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<std::tuple<uint16_t, bool>>& indexVec)
{
    os << "[ ";
    if (!indexVec.empty()) {
        auto tup = indexVec[0];
        os << "{ first: " << std::get<0>(tup) << ", second: " << std::get<1>(tup) << " }";
    }
    for (size_t i = 0; i < indexVec.size(); i++) {
        auto tup = indexVec[i];
        os << ", { first: " << std::get<0>(tup) << ", second: " << std::get<1>(tup) << " }";
    }
    os << " ]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Animal& input)
{
    os << "{ ";
    os << "numFeet: " << input.numFeet;
    os << ", name: " << input.name;
    os << ", measures: " << input.measures;
    os << " }";
    return os;
}
