#ifndef __PROPTEST_STREAM_HPP__
#define __PROPTEST_STREAM_HPP__

#include "testing/Seq.hpp"

namespace PropertyBasedTesting
{

template <typename T>
class Stream : public Seq<T> {
    using Element = T;

    template <typename ...ARGS>
    static Stream fromValues(ARGS...args) {

    }

};

} // namespace

#endif

