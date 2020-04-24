#include "testbase.hpp"

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

std::ostream& operator<<(std::ostream& os, const std::vector<Foo>& input)
{
    os << "[ ";
    for (auto const& i : input) {
        os << i.a << " ";
    }
    os << "]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<int>& input)
{
    os << "[ ";
    for (auto const& i : input) {
        os << i << " ";
    }
    os << "]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<int8_t>& input)
{
    os << "[ ";
    for (auto const& i : input) {
        os << static_cast<int>(i) << " ";
    }
    os << "]";
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

std::ostream& operator<<(std::ostream& os, const std::set<int>& input)
{
    os << "{ ";
    if (input.size() == 1) {
        os << *input.begin();
    } else if (input.size() > 0) {
        os << *input.begin();
        auto second = input.begin();
        second++;
        for (auto itr = second; itr != input.end(); ++itr) {
            os << ", " << *itr;
        }
    }

    os << " }";
    return os;
}