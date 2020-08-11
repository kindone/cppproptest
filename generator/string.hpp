#pragma once
#include "../gen.hpp"
#include "../util/utf8string.hpp"
#include "../util/utf16string.hpp"
#include "../util/cesu8string.hpp"
#include <string>

namespace proptest {

template <>
class PROPTEST_API Arbi<std::string> final : public ArbiContainer<std::string> {
    using ArbiContainer<std::string>::minSize;
    using ArbiContainer<std::string>::maxSize;

public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbi();
    Arbi(Arbi<char>& _elemGen);
    Arbi(GenFunction<char> _elemGen);

    Shrinkable<std::string> operator()(Random& rand) override;
    // FIXME: turn to shared_ptr
    GenFunction<char> elemGen;
};

}  // namespace proptest
