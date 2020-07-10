#pragma once
#include <iostream>
#include <ios>
#include <iomanip>

namespace proptest {
namespace util {

class IosFlagSaver {
public:
    explicit IosFlagSaver(std::ostream& _ios);

    ~IosFlagSaver();

    IosFlagSaver(const IosFlagSaver& rhs) = delete;
    IosFlagSaver& operator=(const IosFlagSaver& rhs) = delete;

private:
    std::ostream& ios;
    std::ios::fmtflags f;
};

}  // namespace util
}  // namespace proptest
