#ifndef __PROPTEST_SEQ_HPP__
#define __PROPTEST_SEQ_HPP__

namespace PropertyBasedTesting
{

template <typename T>
class Seq {
    using Element = T;

    
};

template <typename T>
class Iterator {
    virtual T next();
};

} // namespace

#endif

