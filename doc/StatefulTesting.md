# Stateful Testing

While property-based testing suits well with functions and stateless objects, it's possible to test various state changes with ease. The idea of stateful testing with `cppproptest` is to define and utilize generators for *state changes*.

There are two styles of stateful testing - one with *action functions(lambda)* and one with *action classes*.

While the first style using functions are easier to use and understand, the second style is more formal and traditional way of doing stateful testing. You may choose to use either style. Both have common process of defining and running stateful tests:

1. Define action generator: Define `action`s that each represents unit of state change - e.g. calling `multiply()` method with a numeric multiplier as an argument
2. Create action list generator: Create a generator for the `action` types that can build a list of actions and pass arguments to the selected actions
3. Run the stateful test with randomized action lists generated.

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

You first need to define actions for each state change. This varies between the two styles.

## Stule 1: Using Action Functions

In the first style using functions, an `Action` or a `SimpleAction` is formally defined as a function:

```cpp
template <typename ObjectType>
using Action = function<bool(ObjectType&, ModelType&)>;

template <typename ObjectType>
using SimpleAction = function<bool(ObjectType&)>;
```

Note that `ObjectType` refers to the type of the stateful object of our concern. `ModelType` is the type of additional object with which we'd like to check our stateful object. This additional object is called a *model*. For example, you can mark number of elements in a model to track the inserted or removed elements in a container object. 

### `SimpleAction` - Working without a model

You can use `SimpleAction` and its variant if you do not intend to use a model object. Let's discuss this simper variant first.

The function takes a `ObjectType` reference and returns a `boolean` value as a result. You will typically be defining a `SimpleAction` with lambda. 

Our first goal is to create a generator for our action. A generator for an action with no arguments such as `pop_back()` can be defined as:

```cpp
#include "statefultest.hpp"

auto popBackGen = just<SimpleAction<MyVector>>([](MyVector& obj) {
    obj.pop_back(); 
    return true;
});
```

Notice the usage of `just` generator combinator which will always generate the same lambda. Compare with the `push_back()`'s action generator that requires an integer argument:

```cpp
auto pushBackGen = Arbi<int>().map<SimpleAction<MyVector>>([](int value) {
    return [value](MyVector& obj) {
        obj.push_back(value);
        return true;
    };
});
```

Here you can see an integer generator is transformed as an action generator. The outer lambda returns an action (a function) that calls `push_back()` with the integer argument `value`. 

In both examples, we are returning `true`, indicating the precondition for the action. This is mainly useful when you want add restriction in selecting a state change at a specific object state. For example, you may want to avoid a `pop_back()` to be called on an empty vector, by returning `false` instead:

```cpp
        // ...
        if(obj.size() == 0)
            return false;
        obj.pop_back();
        return true;
        // ...
```

Also, you can add various assertions in the lambda. These will work as postcondition of the action. Any failed assertion will be reported and analyzed, as in ordinary property tests.

With each action generator defined, we will call `actionListGenOf<ObjectType>()` to get a generator for a `std::list<SimpleList>`, which is the random list of actions.

```cpp
auto actionListGen = actionListGenOf(pushBackGen, popBackGen, clearGen); 
```

Finally, we can define a stateful property by calling `statefulProperty<ObjectType>()`. This method requires an initial state generator, and the `actionListGen` we've just obtained. Calling `statefulProperty::go` will execute the stateful property test. 

```cpp
// we can generate initial object from an arbitrary, assuming we have an Arbi<MyVector> defined
auto prop = statefulProperty<T>(
    /* initial state generator */ Arbi<MyVector>(),
    /* action list generator */ actionListGen);
prop.go();

// ...

// or, we can just initialize the object to an empty object
auto prop = statefulProperty<T>(
    /* initial state generator */ just<MyVector>([]() { return MyVector(); }),
    /* action list generator */ actionListGen);
prop.go();


```

### `Action` - Working with a model

If you need a model for advanced tracking of state changes, use `Action` instead of `SimpleAction`. `Action` takes additional parameter indicating the model.

```cpp
template <typename ObjectType>
using Action = function<bool(ObjectType&, ModelType&)>;
```

```cpp
// our simple model that tracks number of elements
struct Counter {
    Counter(int n) : num(n) {}
    int num;
};

auto popBackGen = just<SimpleAction<MyVector, Counter>>([](MyVector& obj, Counter& counter) {
    obj.pop_back(); 
    counter.num--;
    return true;
});
```

You can use `actionListGenOf<ObjectType, ModelType>` to get a action list generator.

```cpp
auto actionListGen = actionListGenOf(pushBackGen, popBackGen, clearGen); 
```

Finally, we can define a stateful property by calling `statefulProperty<ObjectType,ModelType>()`. This method requires an initial state generator, and the `actionListGen` we've just obtained. Compared to `SimpleAction` case, it additionally requires a model factory in the form of `ObjectType& -> ModelType`. This factory is to induce initial model from initial object. Calling `statefulProperty::go()` will execute the stateful property test. 

```cpp
auto prop = statefulProperty<T>(
    /* initial state generator */ Arbi<MyVector>(),
    /* model factory */ [](MyVector& vec) { return Counter(vec.size()); }, 
    /* the action list generator */ actionListGen);
prop.go();
```



## Style 2: Using Action Classes

See [the separate page](./StatefulTestingStyle2.md) for detail. Second style is more traditional way of defining actions. Both style are similar in terms of expressive power.


# Further topics

* See [Concurrency Testing](./ConcurrencyTesting.md) for testing for concurrent changes of a stateful object.
