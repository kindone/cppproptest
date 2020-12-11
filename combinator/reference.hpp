#pragma once

#include "../util/std.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../gen.hpp"

namespace proptest {

template <typename GEN, typename T = typename result_of<GEN(Random&)>::type::type>
auto reference(GEN&& gen) -> Generator<T>
{
    return generator([&gen](Random& rand) {
        return util::forward<GEN>(gen)(rand);
    });
}

} // namespace proptest
