#pragma once
#include "proptest/gen.hpp"
#include "proptest/util/utf8string.hpp"
#include "proptest/util/utf16string.hpp"
#include "proptest/util/cesu8string.hpp"
#include "proptest/util/std.hpp"

/**
 * @file string.hpp
 * @brief Arbitrary for string
 */
namespace proptest {

/**
 * @ingroup Generators
 * @brief Arbitrary for string with configurable character generators and min/max sizes
 */
template <>
class PROPTEST_API Arbi<string> final : public ArbiContainer<string> {
    using ArbiContainer<string>::minSize;
    using ArbiContainer<string>::maxSize;

public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi();
    Arbi(Arbi<char>& _elemGen);
    Arbi(GenFunction<char> _elemGen);

    Shrinkable<string> operator()(Random& rand) override;
    // FIXME: turn to shared_ptr
    GenFunction<char> elemGen;
};

}  // namespace proptest
