#include "proptest/api.hpp"
#include "proptest/util/std.hpp"
#include "proptest/assert.hpp"

namespace proptest {
namespace util {

ostream& errorOrEmpty(bool error)
{
    static stringstream str;
    static ostream& empty = str;
    if (error)
        return cerr;
    else
        return empty;
}

}  // namespace util

AssertFailed::~AssertFailed() {}

PropertyFailedBase::~PropertyFailedBase() {}

Discard::~Discard() {}

Success::~Success() {}

}  // namespace proptest
