# Stateful Testing

While property-based testing suits well with functions and stateless objects, it's also useful in testing for various state changes with ease. Typical properties we can test with stateful tests are as following:

* Test for consistency of internal state
* Test for memory leaks
* Test for concurrent accesses (see [Concurrency Testing](./ConcurrencyTesting.md) for more)

The key idea of stateful testing with `cppproptest` is to generate *state changes*.

1. Define action generators: Define `action`s that each represents unit of state change - e.g. For a numeric object, calling `.multiply(int multiplier)` method with a numeric multiplier as an argument, calling `.divide(int divisor)` method, etc.
2. Build an action list generator: we then need a generator for the `action` types that can build a list of actions and pass required arguments to the selected actions
3. Run the stateful test

Say, you are to write stateful test for your `MyVector`, which is a linear container for integers.

```cpp
class MyVector {
    void push_back(int val);
    int pop_back();
    int size();
    int& at(int pos);
    void clear();
};
```

You first need to define actions for each state change.

## Using Action Functions

An `Action` or a `SimpleAction` is formally defined as a functor object of the form:

*`Action<ObjectType,ModelType>`:*
```javascript
(ObjectType&, ModelType&) -> void
```

*`SimpleAction<ObjectType>`:*
```javascript
(ObjectType&) -> void
```

`ObjectType` refers to the type of the stateful object of our concern. `ModelType` indicates an optional object type with which we'd like to check our stateful object. This additional object is called a *model*. For example, you can mark number of elements in a model to track the inserted or removed elements in a container object. Or you could closely compare your object with an already validated implementation that works similar to yours.

### Option 1: `SimpleAction` - Working without a model

```javascript
(ObjectType&) -> void
```

You can use `SimpleAction` and its variant if you do not intend to use a model object. Let's discuss this simper variant first. The function takes an `ObjectType` reference. You will typically be defining a `SimpleAction` with a lambda. Our first goal is to create a generator for our action. A generator for an action with no arguments such as `pop_back()` can be defined as:

```cpp
#include "statefultest.hpp"

// ...

auto popBackGen = just(SimpleAction<MyVector>([](MyVector& obj) {
    obj.pop_back();
}));
```

Notice the usage of `just` generator combinator which will always generate the same action. Compare with following `push_back()`'s action generator that requires an integer argument:

```cpp
auto pushBackGen = Arbi<int>().map<SimpleAction<MyVector>>([](int value) {
    return SimpleAction<MyVector>([value](MyVector& obj) {
        obj.push_back(value);
    });
});
```

Here you can see an integer generator is transformed as an action generator. The outer lambda returns an action that calls `push_back()` with the integer argument `value`.

You can add various assertions in the action. Any failed assertion will be reported and analyzed, as in ordinary property tests.

With each action generator defined, we would typically combine these generators as one, using `oneOf` combinator:

```cpp
auto actionGen = oneOf<SimpleAction<MyAction>>(pushBackGen, popBackGen, clearGen);
```

This will generate either of 3 actions, with evenly distributed probability (1/3).

Finally, we can define a stateful property by calling `statefulProperty<ObjectType>()`. This method requires an initial state generator, and the `actionGen` we've just obtained. Calling `statefulProperty::go()` will execute the stateful property test.

```cpp
// we can generate initial object from an arbitrary, assuming we have an Arbi<MyVector> defined
auto prop = statefulProperty<T>(
    /* initial state generator */ Arbi<MyVector>(),
    /* action generator */ actionGen);
prop.go();

// ...

// or, we can just initialize the object to an empty object
auto prop = statefulProperty<T>(
    /* initial state generator */ just<MyVector>([]() { return MyVector(); }),
    /* action generator */ actionGen);
prop.go();
```

#### Putting it together:

```cpp
class MyVector {
    void push_back(int val) { ... }
    int pop_back() { ... }
    int size() { ... }
    int& at(int pos) { ... }
    void clear() { ... }
};

TEST(MyVectorTest, Stateful)
{
    auto popBackGen = just(SimpleAction<MyVector>([](MyVector& obj) {
        if(obj.size() == 0)
            return;
        int size = obj.size();
        obj.pop_back();
        PROP_ASSERT(obj.size() == size - 1);
    }));

    auto pushBackGen = Arbi<int>().map<SimpleAction<MyVector>>([](int value) {
        return [value](MyVector& obj) {
            int size = obj.size();
            obj.push_back(value);
            PROP_ASSERT(obj.size() == size + 1);
        };
    });

    auto clearGen = just(SimpleAction<MyVector>([](MyVector& obj) {
        obj.clear();
        PROP_ASSERT(obj.size() == 0);
    }));

    auto actionGen = oneOf<SimpleAction<MyVector>>(pushBackGen, popBackGen, weightedGen<SimpleAction<MyVector>>(clearGen, 0.1));
    // `oneOf` can take weights, so you can adjust rate of generation of an action
    //    auto actionGen = oneOf<SimpleAction<MyVector>>(pushBackGen, popBackGen, weightedGen<SimpleAction<MyVector>>(clearGen, 0.1));
    auto prop = statefulProperty<MyVector>(
        /* initial state generator */ just<MyVector>([]() { return MyVector(); }),
        /* action generator */ actionGen);
    // Tests massive cases with randomly generated action sequences
    prop.go();
}
```

### Option 2: `Action` - Working with a model

If you need a model for advanced tracking of state changes, use `Action` instead of `SimpleAction`. `Action` takes additional parameter indicating the model type. Let's define our model for tracking number of elements for `MyVector`

```cpp
// our simple model that tracks number of elements
struct Counter {
    Counter(int n) : num(n) {}
    int num;
};
```

With this defined, we can continue defining our actions.

```cpp
auto popBackGen = just(Action<MyVector, Counter>([](MyVector& obj, Counter& counter) {
    if(obj.size() == 0)
        return;
    obj.pop_back();
    counter.num--;
}));
```

You can use `oneOf<Action<ObjectType, ModelType>>` to get the combined action generator:

```cpp
auto actionGen = oneOf<Action<MyVector, Counter>>(pushBackGen, popBackGen, clearGen);
```

Finally, we can define a stateful property by calling `statefulProperty<ObjectType,ModelType>()`. This method requires an initial state generator, and the `actionGen` we've just obtained. Compared to `SimpleAction` case, it additionally requires a model factory in the form of `ObjectType& -> ModelType`. This factory is to induce initial model from initial object. Calling `statefulProperty::go()` will execute the stateful property test.

```cpp
auto prop = statefulProperty<T>(
    /* initial state generator */ Arbi<MyVector>(),
    /* model factory */ [](MyVector& vec) { return Counter(vec.size()); },
    /* action generator */ actionGen);
prop.go();
```

While the model in this example is simple, you may choose to use more complex ones. It's often a clever idea to use an existing, well validated implementation as model. For example, we could use `std::vector<int>` as model and perform the actions on both `MyVector` and `std::vector`. We assure `MyVector` works correctly by comparing element-wise with the `std::vector` model object that has undergone the same state changes.

#### Putting it together:

```cpp
class MyVector {
    void push_back(int val) { ... }
    int pop_back() { ... }
    int size() { ... }
    int& at(int pos) { ... }
    void clear() { ... }
};

// our simple model that tracks number of elements
struct Counter {
    Counter(int n) : num(n) {}
    int num;
};

TEST(MyVectorTest, Stateful)
{
    auto popBackGen = just(Action<MyVector, Counter>([](MyVector& obj, Counter& cnt) {
        if(obj.size() == 0)
            return;
        obj.pop_back();
        cnt.num--;
        PROP_ASSERT(cnt.num == obj.size());
    }));

    auto pushBackGen = Arbi<int>().map<Action<MyVector, Counter>>([](int value) {
        return [value](MyVector& obj) {
            obj.push_back(value);
            cnt.num++;
            PROP_ASSERT(cnt.num == obj.size());
        };
    });

    auto clearGen = just(Action<MyVector, Counter>([](MyVector& obj) {
        obj.clear();
        cnt.num = 0;
        PROP_ASSERT(cnt.num == obj.size());
    }));

    // combine action generators
    auto actionGen = oneOf<Action<MyVector, Counter>>(pushBackGen, popBackGen, clearGen);
    // oneOf() can take weights, so you can adjust rate of generation of an action
    //    auto actionGen = oneOf<Action<MyVector, Counter>>(pushBackGen, popBackGen, weightedGen<Action<MyVector, Counter>>(clearGen, 0.1));
    auto prop = statefulProperty<MyVector, Counter>(
        /* initial state generator */ Arbi<MyVector>(),
        /* initial model factory */ [](MyVector& vec) { return Counter(vec.size()); },
        /* action generator */ actionGen);
    // Tests massive cases with randomly generated action sequences
    prop.go();
}
```

### Debugging stateful test failures

A stateful test is succesful if all tried combinations were complete without issues. On the other hand, a failed assertion or an unexpected exception would end up with a stateful test failure. The framework will print the failed condition and tried input combinations so that you can debug the failure. Among the `args`, the first arg is the initial state, and the second one is the action list:

```Shell
Falsifiable, after 12 tests: vec.size() == count (test/test_state_func.cpp:111)
  with args: { [ 1882384569, -1157159508, ..., 128, 32768, 840506558 ], [ Action<?>, Action<?>, Action<?>, ..., Action<?> ] }
```

Note that, by default, an `Action` or a `SimpleAction` has no distinctive description. This is why there are indistinguishable `Action<?>`s printed in the action list. This can be avoided by prepending a description to each action constructor:

```cpp
// action with no argument
auto clearGen = just(SimpleAction<MyVector>>("Clear", [](MyVector& obj) {
    // ...
}));

// action with arguments can be printed nicely with a stringstream
auto pushBackGen = Arbi<int>().map<SimpleAction<MyVector>>([](int value) {
    std::stringstream str;
    str << "PushBack(" << value << ")";
    return SimpleAction(str.str(), [value](MyVector& obj) {
        // ...
    });
});
```

Now you can see the actions are correctly printed:

```Shell
Falsifiable, after 1 tests: vec.size() < 5 (test/test_state_func.cpp:111)
  with args: { [ 1882384569, -1157159508, ..., 128, 32768, 840506558 ], [ PushBack(1894834799), PopBack, Clear, ..., PushBack(814265512) ] }
```


### Configuring stateful test runs

You can alter some of test characteristics of stateful test runs.

* Random seed
* Number of runs
* Maximum time duration of test runs

```cpp
auto prop = statefulProperty(...);
// set random seed
prop.setSeed(5464561L);
// number of sequences to be tested
prop.setNumRuns(10000);
// maximum time duration for go() is 60 seconds
prop.setMaxDurationMs(60*1000);
prop.go();
// or you can simply chain the property:
prop.setSeed(0).setNumRuns(1000).setMaxDurationMs(10000).go();
```

## Alternative Style: Using Action Classes

There are actually two styles of stateful testing - one with *action functions(lambda)* and one with *action classes*. While the first style using functions are easier to use and understand, the second style is more formal way of doing stateful testing. You may choose to use either style. Both have similar process of defining and running stateful tests. See [the separate page](./StatefulTestingStyle2.md) for detail. Both styles are similar in terms of expressive power.


# Further topics

* See [Concurrency Testing](./ConcurrencyTesting.md) for testing for concurrent changes of a stateful object.
