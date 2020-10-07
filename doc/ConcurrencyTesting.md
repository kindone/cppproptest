## Concurrency testing

Once you're familiar with [stateful testing](./StatefulTesting.md), you can get acquanted with concurrent stateful testing easily.
Concurrency testing performs interleaved state transitions using multiple threads in parallel to see if any anomaly or breaking of requirement are present.

Actually, a concurrency test is almost immediately achievable when you have prepared a stateful test for an object type.
Following depicts a concurrent test for `std::vector<int>` with `push_back(int)`, `pop_back()`, and `clear()` actions.


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
concurrentProp.check(/*optional post-check*/);
```

Notice that instead of using `statefulProperty()::forAll()`, we're using `concurrency()::check()`. all other parts are the same as usual stateful test.

Of course, this test would lead to various exceptions or crashes, as `std::vector` is not made for concurrent writes, unless some synchronization mechanism is present.

You can also add a post-check to be performed after each concurrent test run, by adding a function as an argument to `check()`:

```cpp
concurrentProp.check([](std::vector<int>& obj) {
    // ... perform some consistency check for obj
});

// variant with a model
concurrentProp.check([](std::vector<int>& obj, VectorModel& model) {
    // ... perform some consistency check for obj against model
});
```

In concurrent tests, you should be cautious about validation. Your model object as well as the stateful object can be concurrently accessed. So a post-check comes handy, as you don't need to care about synchronization since it's performed after all actions are finished.
