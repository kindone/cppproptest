#pragma once

#include "../util/std.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../gen.hpp"

namespace proptest {

template <typename GEN>
auto reference(GEN&& gen) -> Generator<typename invoke_result_t<GEN, Random&>::type>
{
    return generator([&gen](Random& rand) { return util::forward<GEN>(gen)(rand); });
}

}  // namespace proptest
