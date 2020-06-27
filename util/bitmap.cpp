#include <memory>
#include <atomic>
#include <exception>
#include "bitmap.hpp"

namespace PropertyBasedTesting {

Bitmap::Bitmap()
{
    for (int i = 0; i < size; i++) {
        states[i] = Available;
    }
}

void Bitmap::setChanging(int n)
{
    State AvailableState = Available;
    State UnavailableState = Unavailable;
    // cas
    states[n].compare_exchange_strong(AvailableState, Changing) ||
        states[n].compare_exchange_strong(UnavailableState, Changing);
}

void Bitmap::take(int n)
{
    // assert
    if (states[n] != Changing)
        throw std::runtime_error("invalid state");
    states[n] = Unavailable;
}

void Bitmap::put(int n)
{
    if (states[n] != Changing)
        throw std::runtime_error("invalid state");
    states[n] = Available;
}

int Bitmap::occupyAvailable(int n)
{
    for (int i = n; i < size; i++) {
        State AvailableState = Available;
        if (states[i].compare_exchange_strong(AvailableState, Changing))
            return i;
    }

    for (int i = 0; i < n; i++) {
        State AvailableState = Available;
        if (states[i].compare_exchange_strong(AvailableState, Changing))
            return i;
    }
    return -1;
}

int Bitmap::occupyUnavailable(int n)
{
    for (int i = n; i < size; i++) {
        State UnavailableState = Unavailable;
        if (states[i].compare_exchange_strong(UnavailableState, Changing))
            return i;
    }

    for (int i = 0; i < n; i++) {
        State UnavailableState = Unavailable;
        if (states[i].compare_exchange_strong(UnavailableState, Changing))
            return i;
    }
    return -1;
}

}  // namespace PropertyBasedTesting
