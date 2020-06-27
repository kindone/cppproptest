#include <memory>
#include <atomic>
#include <exception>

namespace PropertyBasedTesting {

struct Bitmap
{
    static constexpr int size = 1000;

    typedef enum
    {
        Available = 0,
        Changing = 1,
        Unavailable = 2
    } State;

    Bitmap();
    void setChanging(int n);
    void take(int n);
    void put(int n);

    int occupyAvailable(int n);
    int occupyUnavailable(int n);

    std::atomic<State> states[size];
};

}  // namespace PropertyBasedTesting