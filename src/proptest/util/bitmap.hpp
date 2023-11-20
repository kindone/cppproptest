#pragma once
#include "proptest/api.hpp"
#include "proptest/util/std.hpp"
#include <atomic>

namespace proptest {
namespace util {

struct PROPTEST_API Bitmap
{
    static constexpr int size = 10000;

    typedef enum
    {
        Available = 0,
        Changing = 1,
        Unavailable = 2
    } State;

    Bitmap();
    Bitmap(const Bitmap&);

// private:
    void setChanging(int n);
    void setChangingFromAcquired(int n);
    void setChangingFromUnacquired(int n);
    void take(int n);
    void put(int n);
    void reset();

    int occupyAvailable(int n);
    int occupyUnavailable(int n);
public:
    int acquire();
    int tryAcquire();
    void unacquire(int n);

// private:
    std::atomic<State> states[size];

};

}  // namespace util
}  // namespace proptest
