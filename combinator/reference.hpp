#pragma once

#include "../util/std.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../gen.hpp"

/** @file reference.hpp
 * @brief Generator combinator for generating a value from a reference of generator by referencing it lazily
 **/

namespace proptest {

/**
 * @ingroup Combinators
 * @brief Takes reference of a generator and generates values from the generator by lazily referencing it
 * @details This can be useful when a generator has to be defined in recursive fashion. e.g. a Tree node generator would
 * need to take the generator itself as argument, but eagerly evaluating it would cause compile error.
 * @code
 * // A Box that can contain 0~2 child boxes can be generated.
 * GenFunction<Box> boxGen = construct<Box, vector<Box>>(Arbi<std::vector<Box>>(reference(boxGen)).setSize(0, 2));
 *
 * @endcode
 */
template <typename GEN>
auto reference(GEN&& gen) -> Generator<typename invoke_result_t<GEN, Random&>::type>
{
    return generator([&gen](Random& rand) { return util::forward<GEN>(gen)(rand); });
}

}  // namespace proptest
