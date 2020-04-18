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

Many primitive types and containers have their Aribtrary<T> defined by the framework for convenient use.

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

### Predefined Arbitraries

The framework provides `Arbitrary<T>` for following primitive types 
* Integral types: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
* Floating point types: `float`, `double`
* String types: `std::string`, `UTF8String`
* Standard containers: `std::vector`, `std::pair`, `std::tuple`

### Generator Combinators

Generator combinators are provided for building a new generator based on existing ones.
* `pair<T1, T2>(gen1, gen2)` : generates a `std::pair<T1,T2>` based on result of generators `gen1` and `gen2`
```cpp
auto pairGen = pair<int, std::string>(Arbitrary<int>(), Arbitrary<std::string>());
```

* `tuple<T1, ..., TN>(gen1, ..., genN)`: generates a `std::tuple<T1,...,TN>` based on result of generators `gen1` through `genN`
```cpp
auto tupleGen = tuple<int, std::string, double>(Arbitrary<int>(), Arbitrary<std::string>(), Arbitrary<double>());
```

* `filter<T>(gen, condition_predicate)`:  generates a type `T` that satisfies condition predicate (`condition_predicate` returns `true`) 
```cpp
// generates even numbers
auto evenGen = filter<int>(Arbitrary<int>(),[](const int& num) {
    return num % 2 == 0;
});
```

* `transform<T,U>(gen, transformer)`: generates type `U` based on generator for type `T`, using `transformer` that transforms a value of type `T` to type `U`
```cpp
// generates string from integers (e.g. "0", "1", ... , "-16384")
auto numStringGen = transform<int, std::string>(Arbitrary<int>(),[](const int& num) {
    return std::string(num);
});
```

* `oneOf<T>(gen1, ..., genN)`: generates a type `T` from multiple generators for type `T`, by choosing one of the generators randomly
```cpp
// returns a numeric within ranges (0,10), (100, 1000), (10000, 100000)
auto evenGen = oneOf<int>(inRange<int>(0, 10), inRange<int>(100, 1000), inRange<int>(10000, 100000));
```

* `dependency<T,U>(gen1, gen2generator)`: generates a `std::pair<T,U>` with a generator `gen1` for type `T` and `gen2generator`. `gen2generator` receives a type `T` and returns a generator for type `U`.
```cpp
auto sizeAndVectorGen = dependency<int, std::vector<bool>>(Arbitrary<int>(),[](const int& num) {
    auto vectorGen = Arbitrary<std::vector<int>>();
    vectorGen.maxLen = num;
    // generates a vector with maximum size of num
    return vectorGen;
});
```


## Shrinking

## Stateful testing

## Concurrency testing

## Generating Mocks
## 
