#pragma once
#include "../gen.hpp"
#include "../util/utf8string.hpp"
#include "../util/utf16string.hpp"
#include "../util/cesu8string.hpp"
#include "../util/std.hpp"

namespace proptest {

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
