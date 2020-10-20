# Stateful Testing

While property-based testing suits well with functions and stateless objects, it's also useful in testing for various state changes with ease. Typical properties we can test with stateful tests are as following:

* Test for consistency of internal state
* Test for memory leaks
* Test for concurrent accesses (see [Concurrency Testing](./ConcurrencyTesting.md) for more)

The key idea of stateful testing with `cppproptest` is to generate *state changes*.

1. Define action generators: Define `action`s that each represents unit of state change - e.g. For a numeric object, calling `.multiply(int multiplier)` method with a numeric multiplier as an argument, calling `.divide(int divisor)` method, etc.
2. Define an action list generator: we then need a generator for the `action` types that can build a list of actions and pass required arguments to the selected actions
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

An `Action` or a `SimpleAction` is formally defined as a function:

```cpp
template <typename ObjectType>
using Action = function<bool(ObjectType&, ModelType&)>;

template <typename ObjectType>
using SimpleAction = function<bool(ObjectType&)>;
```

`ObjectType` refers to the type of the stateful object of our concern. `ModelType` is the type of additional object with which we'd like to check our stateful object. This additional object is called a *model*. For example, you can mark number of elements in a model to track the inserted or removed elements in a container object.

### Option 1: `SimpleAction` - Working without a model

```cpp
template <typename ObjectType>
using SimpleAction = function<bool(ObjectType&)>;
```

You can use `SimpleAction` and its variant if you do not intend to use a model object. Let's discuss this simper variant first. The function takes a `ObjectType` reference and returns a `boolean` value as a result. You will typically be defining a `SimpleAction` with lambda. Our first goal is to create a generator for our action. A generator for an action with no arguments such as `pop_back()` can be defined as:

```cpp
#include "statefultest.hpp"

// ... 

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
    auto popBackGen = just<SimpleAction<MyVector>>([](MyVector& obj) {
        if(obj.size() == 0)
            return false;
        obj.pop_back(); 
        return true;
    });

    auto pushBackGen = Arbi<int>().map<SimpleAction<MyVector>>([](int value) {
        return [value](MyVector& obj) {
            obj.push_back(value);
            return true;
        };
    });

    auto clearGen = just<SimpleAction<MyVector>>([](MyVector& obj) {
        obj.clear();
        return true;
    });

    auto actionListGen = actionListGenOf<MyVector>(pushBackGen, popBackGen, weightedGen<SimpleAction<MyVector>>(clearGen, 0.1)); 
    // actionListGenOf is an `oneOf` generator that can take weights, so you can adjust rate of generation of an action with a weight
    //    auto actionListGen = actionListGenOf<MyVector>(pushBackGen, popBackGen, weightedGen<SimpleAction<MyVector>>(clearGen, 0.1)); 
    auto prop = statefulProperty<MyVector>(just<MyVector>([]() { return MyVector(); }), actionListGen);
    // Tests massive cases with randomly generated action sequences
    prop.forAll();
}
```

### Option 2: `Action` - Working with a model

If you need a model for advanced tracking of state changes, use `Action` instead of `SimpleAction`. `Action` takes additional parameter indicating the model.

```cpp
template <typename ObjectType>
using Action = function<bool(ObjectType&, ModelType&)>;
```

And Let's define our model for tracking number of elements for `MyVector`

```cpp
// our simple model that tracks number of elements
struct Counter {
    Counter(int n) : num(n) {}
    int num;
};
```

With this defined, we can continue defining our actions.

```cpp
auto popBackGen = just<SimpleAction<MyVector, Counter>>([](MyVector& obj, Counter& counter) {
    if(obj.size() == 0)
        return false;
    obj.pop_back();
    counter.num--;
    return true;
});
```

You can use `actionListGenOf<ObjectType, ModelType>` to get a action list generator:

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
    auto popBackGen = just<Action<MyVector, Counter>>([](MyVector& obj, Counter& cnt) {
        if(obj.size() == 0)
            return false;
        obj.pop_back(); 
        cnt.num--;
        PROP_ASSERT(cnt.num == obj.size());
        return true;
    });

    auto pushBackGen = Arbi<int>().map<Action<MyVector, Counter>>([](int value) {
        return [value](MyVector& obj) {
            obj.push_back(value);
            cnt.num++;
            PROP_ASSERT(cnt.num == obj.size());
            return true;
        };
    });

    auto clearGen = just<Action<MyVector, Counter>>([](MyVector& obj) {
        obj.clear();
        cnt.num = 0;
        PROP_ASSERT(cnt.num == obj.size());
        return true;
    });

    auto actionListGen = actionListGenOf<MyVector, Counter>(pushBackGen, popBackGen, clearGen); 
    // actionListGenOf is an `oneOf` generator that can take weights, so you can adjust rate of generation of an action with a weight
    //    auto actionListGen = actionListGenOf<MyVector, Counter>(pushBackGen, popBackGen, weightedGen<Action<MyVector, Counter>>(clearGen, 0.1));
    auto prop = statefulProperty<MyVector, Counter>(/*initial state generator*/ Arbi<MyVector>(), /*initial model factory*/ [](MyVector& vec) { return Counter(vec.size()); }, actionListGen);
    // Tests massive cases with randomly generated action sequences
    prop.forAll();
}
```

## Alternative Style: Using Action Classes

There are actually two styles of stateful testing - one with *action functions(lambda)* and one with *action classes*. While the first style using functions are easier to use and understand, the second style is more formal and traditional way of doing stateful testing. You may choose to use either style. Both have similar process of defining and running stateful tests. See [the separate page](./StatefulTestingStyle2.md) for detail. This style is more traditional way of defining actions. Both styles are similar in terms of expressive power.


# Further topics

* See [Concurrency Testing](./ConcurrencyTesting.md) for testing for concurrent changes of a stateful object.
