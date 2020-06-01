#pragma once
#include "../gen.hpp"
#include "../util/utf8string.hpp"
#include "../util/utf16string.hpp"
#include "../util/cesu8string.hpp"
#include <string>

namespace PropertyBasedTesting {

template <>
class PROPTEST_API Arbitrary<std::string> final : public Gen<std::string> {
public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary();
    Arbitrary(Arbitrary<char>& _elemGen);
    Arbitrary(std::function<Shrinkable<char>(Random&)> _elemGen);

    Shrinkable<std::string> operator()(Random& rand) override;

    Arbitrary setMinSize(size_t size);
    Arbitrary setMaxSize(size_t size);
    Arbitrary setSize(size_t size);

    // FIXME: turn to shared_ptr
    std::function<Shrinkable<char>(Random&)> elemGen;
    size_t minSize;
    size_t maxSize;
};

}  // namespace PropertyBasedTesting
