# Concurrency Testing

Once you're familiar with [stateful testing](./StatefulTesting.md), you can get acquainted with concurrent stateful testing easily.
Concurrency testing performs interleaved state transitions using multiple threads in parallel. It allows us to see if any anomaly or breaking of requirement can be found in the tested component with concurrent accesses.

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

You may have noticed that, above test would lead to exceptions or crashes, as `std::vector` is not made for concurrent writes, unless some synchronization mechanism is present.

You can also add a post-check to be performed after each concurrent test run, by setting a post-check function to concurrent property with `setPostCheck()` method:

```cpp
concurrentProp.setPostCheck([](std::vector<int>& obj) {
    // ... post-check. perform some consistency check for obj
}).go();

// variant with a model
concurrentProp.setPostCheck([](std::vector<int>& obj, VectorModel& model) {
    // ... post-check. perform some consistency check for obj against model
}).go();
```

While you can perform checks in some of the actions, it's sometimes better to have a post-check instead. In concurrent tests, your model as well as the stateful object can be concurrently accessed. Adding synchronization primitives for model object can cause unintended serialization to occur on the stateful object, too. This is why a post-check comes handy, as you don't need to care about synchronization since it's performed after all actions are finished and threads are joined.

```cpp
```
