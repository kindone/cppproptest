# proptest
Property-Based Testing Framework for C++


## `property` and `check`

`property` defines a property with optional configuration and `check` is shorthand for`Property::check`.
`Property::check` performs property-based test using given callable (a function, a functor, or a lambda).

```cpp
check([](int a, int b) -> bool {
    PROP_ASSERT(a+b == b+a);
    PROP_STAT(a+b > 0);
    return true;
});
```

is equivalent to

```cpp
property([](int a, int b) -> bool {
    PROP_ASSERT(a+b == b+a);
    PROP_STAT(a+b > 0);
    return true;
}).check();
```

### Defining property
Defining a property requires a callable with following signature:

```cpp
(arg1, ..., argN) -> bool
```

For example, a lambda as following is such callable with an int as parameter:

```cpp
[](int a) -> bool {
    return true;
}
```

Arguments are to be generated automatically by the framework and the return value of the callable indicates success(`true`) or failure(`false`) of a property.
In above case, the callable is called with an integer argument randomly generated for parameter `a`, by the test framework. 

For a property to be defined, the framework requires either an `Arbitrary<T>` is defined for a parameter type, or a custom generator is provided. In above example, the predefined generator `Arbitrary<int>` is used to generate an integer argument.

You can supply a custom generator as additional arguments to `property()` function, as following.

```cpp
property([](int a) -> bool {
    return true;
}, myIntGenerator);
```

Many primitive types and containers have their `Aribtrary<T>` defined by the framework for convenient use.

## Generators and Arbitraries

You can use generators to generate randomized arguments for properties.

A generator is a callable with following signature:

```cpp
(Random&) -> Shrinkable<T>
```

You can refer to `Shrinkable` for further detail, but you can treat it as a wrapper for a type T here. The main point is that this generates a value of type T from a random generator. A generator can be defined as functor or lambda, as you prefer.  

```cpp
auto myIntGen = [](Random& rand) {
    int smallInt = rand.getRandomInt8();
    return make_shrinkable<int>(smallInt);
};
```

An `Arbitrary` refers to default generators for a type. You can freely define an `Arbitrary<T>` for your type `T`. By defining an `Arbitrary`, you can omit a custom generator to be passed everytime you define a property for the type. Following shows an example for defining an `Arbitrary`

```cpp
namespace PropertyBasedTesting {

struct Arbitrary<Car> : Gen<Car> {
  Shrinkable<Car> operator()(Random& rand) {
    bool isAutomatic = rand.getRandomBool();
    return make_shrinkable<Car>(isAutomatic);
  }
};

}
```


* [Generators](doc/Generators.md)
* [Shrinking](doc/Shrinking.md)
* [Stateful Testing](doc/statefulTesting.md)
* [Concurrency Testing](doc/ConcurrencyTesting.md)
* [Mocking](doc/Mocking.md)

