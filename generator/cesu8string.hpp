#pragma once
#include "../gen.hpp"
#include "../util/cesu8string.hpp"
#include <string>

namespace proptest {

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
