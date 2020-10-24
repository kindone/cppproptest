#include <memory>
#include <atomic>
#include <exception>
#include "bitmap.hpp"

namespace proptest {
namespace util {

Bitmap::Bitmap()
{
    for (int i = 0; i < size; i++) {
        states[i].store(Available);
    }
}

Bitmap::Bitmap(const Bitmap& other)
{
    for (int i = 0; i < size; i++) {
        states[i].store(other.states[i].load());
    }
}

int Bitmap::acquire() {
    int n = -1;
    do {
        n = occupyAvailable(0);
    } while(n == -1);

    take(n);
    return n;
}

int Bitmap::tryAcquire() {
    int n = occupyAvailable(0);
    if(n != -1)
        take(n);
    return n;
}

void Bitmap::unacquire(int n) {
    setChangingFromAcquired(n);
    put(n);
}

void Bitmap::setChanging(int n)
{
    State AvailableState = Available;
    State UnavailableState = Unavailable;
    // cas
    states[n].compare_exchange_strong(AvailableState, Changing) ||
        states[n].compare_exchange_strong(UnavailableState, Changing);
}

void Bitmap::setChangingFromAcquired(int n)
{
    State UnavailableState = Unavailable;
    // cas
    states[n].compare_exchange_strong(UnavailableState, Changing);
}

void Bitmap::setChangingFromUnacquired(int n)
{
    State AvailableState = Available;
    // cas
    states[n].compare_exchange_strong(AvailableState, Changing);
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

void Bitmap::reset()
{
    for (int i = 0; i < size; i++) {
        states[i] = Available;
    }
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

}  // namespace util
}  // namespace proptest
