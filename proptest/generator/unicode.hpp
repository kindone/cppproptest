#pragma once

#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../api.hpp"
#include "../util/std.hpp"

namespace proptest {

struct PROPTEST_API UnicodeGen
{
    Shrinkable<uint32_t> operator()(Random& rand);
};

}  // namespace proptest
