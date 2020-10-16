#include "assert.hpp"
#include <iostream>
#include <sstream>

namespace proptest {
namespace util {

std::ostream& errorOrEmpty(bool error)
{
    static std::stringstream str;
    static std::ostream& empty = str;
    if (error)
        return std::cerr;
    else
        return empty;
}

}  // namespace util

AssertFailed::~AssertFailed() {}

PropertyFailedBase::~PropertyFailedBase() {}

Discard::~Discard() {}

Success::~Success() {}

}  // namespace proptest
