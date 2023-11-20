#include "proptest/Stream.hpp"

namespace proptest {


Stream::Stream() {
}

Stream::Stream(const Stream& other) : headPtr(other.headPtr), tailGen(other.tailGen) {
}

Stream::Stream(const shared_ptr<Stream>& other) : headPtr(other->headPtr), tailGen(other->tailGen) {
}

Stream& Stream::operator=(const Stream& other)
{
    headPtr = other.headPtr;
    tailGen = other.tailGen;

    return *this;
}

bool Stream::isEmpty() const {
    return !static_cast<bool>(headPtr);
}

Stream Stream::tail() const
{
    if (isEmpty())
        return Stream();

    return Stream((*tailGen)());
}

Stream Stream::concat(const Stream& other) const
{
    if (isEmpty())
        return other;
    else {
        return Stream(headPtr,
                            [tailGen = this->tailGen, other]() { return Stream((*tailGen)()).concat(other); });
    }
}

Stream Stream::take(int n) const
{
    if (isEmpty())
        return empty();
    else {
        auto self = *this;
        if (n == 0)
            return Stream::empty();

        return Stream(headPtr, [self, n]() { return Stream(self.tail()).take(n - 1); });
    }
}

Stream Stream::empty() { return Stream(); }

function<Stream()> Stream::done()
{
    static auto produceEmpty = +[]() -> Stream { return empty(); };
    return produceEmpty;
}

}  // namespace proptest
