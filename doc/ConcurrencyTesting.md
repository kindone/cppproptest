## Concurrency testing

Once you're familiar with [stateful testing](./StatefulTesting.md), you can get acquanted with concurrent stateful testing easily.
Concurrency testing performs interleaved state transitions using multiple threads in parallel to see if any anomaly or breaking of requirement can be found.

Actually, a concurrency test is almost immediately achievable when you have prepared a stateful test for an object type.
Following depicts a concurrent test for `std::vector<int>` with `push_back(int)`, `pop_back()`, and `clear()` actions included.


```cpp

using namespace proptest;
using namespace proptest::concurrent;

// ...

auto pushBackGen = Arbi<int>().map<SimpleAction<std::vector<int>>>([](int& value) {
    return [value](std::vector<int>& obj) {
        obj.push_back(value);
        return true;
    };
});

auto popBackGen = just<SimpleAction<std::vector<int>>>([](std::vector<int>& obj) {
    if (obj.empty())
        return true;
    obj.pop_back();
    return true;
});

auto clearGen = just<SimpleAction<std::vector<int>>>([](std::vector<int>& obj) {
    obj.clear();
    return true;
});

auto actionListGen = actionListGenOf<std::vector<int>>(pushBackGen, popBackGen, clearGen);
auto concurrentProp = concurrency<std::vector<int>>(Arbi<std::vector<int>>(), actionListGen);
concurrentProp.go();
```

You may have noticed that, this test would lead to various exceptions or crashes, as `std::vector` is not made for concurrent writes, unless some synchronization mechanism is present.

You can also add a post-check to be performed after each concurrent test run, by adding a post-check function as an argument to `check()`:

```cpp
concurrentProp.go([](std::vector<int>& obj) {
    // ... post-check. perform some consistency check for obj
});

// variant with a model
concurrentProp.go([](std::vector<int>& obj, VectorModel& model) {
    // ... post-check. perform some consistency check for obj against model
});
```

In concurrent tests, you should be cautious about validation. Your model object as well as the stateful object can be concurrently accessed. Adding synchronization primitives for model object can cause serialization to occur on stateful object, too. This is why a post-check comes handy, as you don't need to care about synchronization since it's performed after all actions are finished.
