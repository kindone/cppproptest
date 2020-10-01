# Stateful Testing

While property-based testing suits well with functions and stateless objects, it's possible to test various state changes with ease. The idea of stateful testing with `cppproptest` is to define and utilize generators for *state changes*.

There are two variants of stateful testing:
1. Defining and testing state changes with *action functions(lambda)*
2. Defining and testing state changes with *action classes*

While the first style using functions are easier to use and understand, the second style is more formal and traditional way of doing stateful tesing. You may choose to use either style. Both have common process of defining and running stateful tests:

1. Define action generator: Define `action`s that each represents unit of state change - e.g. calling `multiply` method with a numeric multiplier as an argument
2. Define action sequence generator: Define a generator for the `action` types that can build a seqeunce of actions and pass arguments to the selected actions
3. Run the stateful test with randomized action sequences generated as in usual property tests

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

In the first style using functions, an `Action` is formally defined as a function:

```cpp
template <typename SystemType>
using Action = function<bool(SystemType&)>;
```

Note that `SystemType` refers to the type of the stateful object of our concern.
The function takes a `SystemType` reference and returns a `boolean` value as a result. You will typically be defining an `Action` with lambda. 

Our goal is to create a generator for our action. A generator for an action with no arguments such as `pop_back` can be defined as:

```cpp
auto popBackGen = just<Action<MyVector>>([](MyVector& obj) {
    obj.pop_back(); 
    return true;
});
```

Notice the usage of `just` generator combinator which will always generate the same lambda. Compare with the `push_back` action generator that requires an integer argument:

```cpp
auto pushBackGen = Arbi<int>().map<Action<MyVector>>([](int value) {
    return [value](MyVector& obj) {
        obj.push_back(value);
        return true;
    };
});
```

Here you can see an integer generator is transformed as an action generator. The outer lambda returns an action (a function) that calls `push_back` with the integer argument `value`. 

In both examples, we are returning `true`, indicating the precondition for the action. This is mainly useful when you want add restriction in selecting a state change at a specific object state. For example, you may want to avoid a `pop_back` to be called on an empty vector, by returning `false` instead:

```cpp
        // ...
        if(obj.size() == 0)
            return false;
        obj.pop_back();
        return true;
        // ...
```

Also, you can add various assertions in the lambda. These will work as postcondition of the action. Any failed assertion will be reported and analyzed, as in ordinary property tests.

Finally, with the action generators defined, we will call `actionProperty<SystemType>::forAll` to run the stateful property test:

```cpp
auto prop = actionProperty<T>(pushBackGen, popBackGen, clearGen);
prop.forAll();
```

## Style 2: Using Action Classes

See [the separate page](./StatefulTestingStyle2.md) for detail. Second style is more traditional way of defining actions. There is no significant difference in the expressive power between the two styles.
