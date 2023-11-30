#pragma once
#include "std.hpp"

namespace proptest {
namespace util {

class IosFlagSaver {
public:
    explicit IosFlagSaver(ostream& _ios);

    ~IosFlagSaver();

    IosFlagSaver(const IosFlagSaver& rhs) = delete;
    IosFlagSaver& operator=(const IosFlagSaver& rhs) = delete;

private:
    ostream& ios;
    ios::fmtflags f;
};

}  // namespace util
}  // namespace proptest
