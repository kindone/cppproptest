#pragma once
#include "../api.hpp"
#include <memory>
#include <atomic>
#include <exception>

namespace pbt {

struct PROPTEST_API Bitmap
{
    static constexpr int size = 1000;

    typedef enum
    {
        Available = 0,
        Changing = 1,
        Unavailable = 2
    } State;

    Bitmap();
    Bitmap(const Bitmap&);
    void setChanging(int n);
    void take(int n);
    void put(int n);
    void reset();

    int occupyAvailable(int n);
    int occupyUnavailable(int n);

    std::atomic<State> states[size];
};

}  // namespace pbt
