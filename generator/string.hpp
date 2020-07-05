#pragma once
#include "../gen.hpp"
#include "../util/utf8string.hpp"
#include "../util/utf16string.hpp"
#include "../util/cesu8string.hpp"
#include <string>

namespace pbt {

template <>
class PROPTEST_API Arbitrary<std::string> final : public ArbitraryContainer<std::string> {
    using ArbitraryContainer<std::string>::minSize;
    using ArbitraryContainer<std::string>::maxSize;
public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary();
    Arbitrary(Arbitrary<char>& _elemGen);
    Arbitrary(std::function<Shrinkable<char>(Random&)> _elemGen);

    Shrinkable<std::string> operator()(Random& rand) override;
    // FIXME: turn to shared_ptr
    std::function<Shrinkable<char>(Random&)> elemGen;
};

}  // namespace pbt
