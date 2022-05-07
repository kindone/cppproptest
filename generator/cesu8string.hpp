#pragma once
#include "../gen.hpp"
#include "../util/cesu8string.hpp"
#include "../util/std.hpp"

/**
 * @file cesu8string.hpp
 * @brief Arbitrary for CESU8String
 */

namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for CESU-8 string with configurable code generator and min/max sizes
 */
template <>
class PROPTEST_API Arbi<CESU8String> final : public ArbiContainer<CESU8String> {
public:
    using ArbiContainer<CESU8String>::minSize;
    using ArbiContainer<CESU8String>::maxSize;
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi();
    Arbi(Arbi<uint32_t>& _elemGen);
    Arbi(GenFunction<uint32_t> _elemGen);

    Shrinkable<CESU8String> operator()(Random& rand) override;

    GenFunction<uint32_t> elemGen;
};

}  // namespace proptest
