# Generating Inputs

## Generators in Property-Based Testing

Property-based testing promotes the concept of **input domain** of a property. In property-based testing, **generators** are the means for representing input domains. A _generator_ is basically a function that generates random values. It defines the constraints of the generated random values in its body. Even a simple `forAll()` call depends on generators under the hood. Let's see following example:

```cpp
forAll([](int age, std::string name) {
});
```

This `forAll()` call takes a function with parameters of types `int` and `std::string`. This function is the *property function*. If no additional specification is given on how to generate the values for `age` and `name` as in this example, the parameter types are identified to invoke the default generators for those types. In this case, it calls the default generators for `int` and `std::string` types. Those default generators are called the *arbitraries*. You can access the arbitraries of a type `T` with `Arbitrary<T>` or `Arbi<T>`. This code is actually equivalent to:

```cpp
forAll([](int age, std::string name) {
}, Arbitrary<int>(), Arbitrary<std::string>());
```

Notice the extra arguments `Arbitrary<int>()` and `Arbitrary<std::string>()` in the `forAll()` call. As you can see, `forAll()` actually requires some information on how to generate the values for the parameter types. Some of the often used types have default generators defined in `cppproptest`.

&nbsp;

## Arbitraries - The Default Generators

### What makes defaults so special

An `Arbitrary<T>` or its alias `Arbi<T>` is a **generator** type (that also coerces to `GenFunction<T>`).
Arbitraries are specially treated in `cppproptest`. An arbitrary serves as globally defined default _generator_ for the type. If a default generator for a type is available, `cppproptest` can use the default generator to generate a value of that type, if no generator has been specified.

```cpp
forAll([](T1 t1, T2 t2, ..., TN tn) {
    // property function body
}, /* custom generators for T1, ..., TN */);
```

For each of the parameter types of a property function, `forAll()` requires either a custom generator is provided as an argument, or a conforming `Arbitrary<T>` class has been defined in `proptest` namespace. A custom generator can be supplied in the `forAll()` function arguments next to the property function, as in the same order of parameters of the property function. If it hasn't been supplied, `forAll()` looks up the default generator - the _arbitrary_ - and uses it instead. If there were no `proptest::Arbitrary<T>` defined, the compilation would fail.

```cpp
// if there is no default generator available, you must provide a generator for the type SomeNewType.
forAll([](SomeNewType x) {
}, someNewTypeGen);


// explicit generators should be supplied in same order as parameter types of property function
forAll([](SomeNewType x, SomeOtherType y) {
}, someNewTypeGen, SomeOtherTypeGen);


// if there is a default generator (Arbitrary<SomeType>) available, you can use that generator by omitting the argument
forAll([](SomeType x) {
});

// Partially specifying generators is also allowed. Other types will be generated with arbitraries
forAll([](SomeNewType x, SomeOtherType y) {
}, someNewTypeGen); // y will be generated with Arbitary<SomeOtherType>
```


### Built-in Arbitraries

`cppproptest` provides a set of built-in generators for generation of types that are often used in practice. These built-in generators are in the form of Arbitraries. You can access an arbitrary for `T` with `proptest::Arbitrary<T>`. Some of them are defined as template classes with type parameters for universality. For example, `Arbitrary<vector<T>>` defines a generator for a vector of any given type `T`, assuming you have an arbitrary for `T` already defined, or you have provided a custom generator for `T` as an argument for the vector arbitrary's constructor. Arbitraries of Commonly used standard containers are defined with type parameters so that you can generate such containers for the elemental types you desire.

Here's quick reference for built-in arbitraries:

| Purpose                          | Generator                                   | Examples                            |
| -------------------------------- | ------------------------------------------- | ----------------------------------- |
| Generate a boolean               | `Arbi<bool>()`                              | `true` or `false`                   |
| Generate a character             | `Arbi<char>()`                              | `'c'` or `'%'`                      |
| Generate an integer              | `Arbi<int>()`, `Arbi<uint64_t>()`, ...      | `12` or `-1133`                     |
| Generate a floating point number | `Arbi<float>()`, `Arbi<double>()`           | `3.4` or `-1.4e3`                   |
| Generate a string                | `Arbi<std::string>()`, `Arbi<UTF8String>()` | `"world"` or `"あ叶葉말"`           |
| Generate a pair                  | `Arbi<std::pair<T1,T2>>()`                  | `{1, "xv"}` or `{true, 3.4}`        |
| Generate a tuple                 | `Arbi<std::tuple<Ts...>>()`                 | `{1, "xv", true}` or `{true, 3.4}`  |
| Generate a list                  | `Arbi<std::list<T>>()`                      | `{10, -4, 0}` or `{"k", "&"}`       |
| Generate a vector                | `Arbi<std::vector<T>>()`                    | `{10, -4, 0}` or `{"k", "&"}`       |
| Generate a set                   | `Arbi<std::set<T>>()`                       | set `{1, 3, 4}` but not `{1, 1, 3}` |
| Generate a map                   | `Arbi<std::map<K,V>>()`                     | map of `"Bob" -> 25, "Alice" -> 30` |

* Boolean type:`bool`
* Character type: `char`
* Integral types: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
* Floating point types: `float`, `double`
* String types:
    * `std::string` (defaults to generate ASCII character strings in \[0x01, 0x7F\] range)
    * `UTF8String` (a class which extends `std::string` and can be used to generate valid [UTF-8](https://en.wikipedia.org/wiki/UTF-8) strings by using `Arbi<UTF8String>`)
    * `CESU8String` (similar to UTF-8, but can be used to generate valid [CESU-8](https://en.wikipedia.org/wiki/CESU-8) strings)
    * `UTF16BEString` and `UTF16LEString` for [UTF-16](https://en.wikipedia.org/wiki/UTF-16) big and little endian strings. CESU-8 and Unicode types produce full unicode code point range of \[0x1, 0x10FFFF\], excluding forbidden surrogate code points (\[0xD800, 0xDFFF\])
* Shared pointers: `std::shared_ptr<T>` where an `Arbi<T>` or a custom generator for `T` is available. It's also useful for generating polymorphic types.
    ```cpp
    struct Action {
        virtual int get() = 0;
    };
    struct Insert : Action {
        virtual void get() { return 1; }
    };
    struct Delete : Action {
        virtual void get() { return 2; }
    };
    Generator<std::shared_ptr<Action>>(...); // can hold both Insert and Delete
    ```
* Standard containers: `std::string`, `std::vector`, `std::list`, `std::set`, `std::pair`, `std::tuple`, `std::map`
    * Arbitraries for containers can optionally take a custom generator for their elemental types. If no custom generator for elemental type `T` is provided, `Arbitrary<T>` will be used instead.
        ```cpp
        // You can supply a specific generator for integers
        auto vecInt0to100 = Arbi<std::vector<int>>(interval<int>(0,100));
        // otherwise, Arbi<int> is used
        auto vecInt = Arbi<std::vector<int>>();

        // string aarbitraries also take optional element generator
        auto uppercaseGen = Arbi<std::string>(interval('A', 'Z'));
        auto alphabetGen = Arbi<std::string>(unionOf(interval('A', 'Z'), interval('a','z')));
        ```

    * `Arbi<std::Map>` provides setter methods for assigning a key generator and a value generator.

        ```cpp
        auto mapGen = Arbi<std::map<int,int>>();
        mapGen.setKeyGen(interval<int>(0,100)); // interval: key ranges from 0 to 100
        mapGen.setElemGen(interval<int>(-100, 100)); // interval: value ranges from -100 to 100
        ```

       * Containers provide methods for configuring the desired sizes
        * `setMinSize(size)`, `setMaxSize(size)` for restricting the container to specific range of sizes
        * `setSize(size)` for restricting the container to a specific size

        ```cpp
        auto vecInt = Arbi<std::vector<int>>();
        vecInt.setSize(10);    // 1) generated vector will always have size 10
        vecInt.setMinSize(1);  // 2) generated vector will have size >= 1
        vecInt.setMaxSize(10); //    generated vector will have size <= 10
        vecInt.setSize(1, 10); // 3) generated vector will have size >= 1 and size <= 10
        ```

As long as a generator for type `T` is available (either by `Arbitary<T>` defined or a custom generator provided), you can generate a container of that type, however complex the type `T` is, even including a container type. This means you can readily generate a random `vector<vector<int>>`, as `Arbitrary<vector<T>>` and `Arbitrary<int>` is readily available.

```cpp
    Arbi<std::vector<std::vector<int>>>(); // generates a vector of vector of ints.
    Arbi<std::map<std::string, std::vector<std::set<int>>>>();
```

This design makes arbitraries of `cppproptest` composable, meaning that they can be easily reusable as building blocks for a new generator.


### Defining an Arbitrary

With template specialization, new `proptest::Arbi<T>` (or its alias `proptest::Arbitrary<T>`) for type `T` can be defined, if it hasn't been already defined yet. By defining an _Arbitrary_, you are effectively adding a default generator for a type.

Following shows an example of defining an _Arbitrary_. Note that it should be defined under `proptest` namespace in order to be recognized and accessible by the library.

```cpp
namespace proptest { // you should define your Arbi<T> inside this namespace

// define a template specialization of Arbi for Car type
// by extending ArbiBase, you are decorating your arbitrary with standard methods (map, flatMap, filter, etc.)
template <>
struct Arbi<Car> : ArbiBase<Car> {
  Shrinkable<Car> operator()(Random& rand) {
    bool isAutomatic = rand.getRandomBool();
    return make_shrinkable<Car>(isAutomatic); // make_shrinkable creates a Car object by calling Car's constructor with 1 boolean parameter
  }
};

}
```

Although you can define an arbitrary as shown in this example, it's only required to do so if you desire to have a default generator for the type.


### Utility methods of Arbitrary

`Arbitrary<T>` provides useful helpers for creating new generators from existing ones[^generatorT]. `filter` is such a helper. It restrictively generates values that satisfy a criteria function. Following shows an even number generator from the integer `Arbitrary`.

```cpp
// generates any integers
auto anyIntGen = Arbi<int>();
// generates even integers
auto evenGen = anyIntGen.filter([](int& num) {
    return num % 2 == 0;
});
```

You can find the full list of such helpers in [Utility methods in standard generators](Combinators.md#utility-methods-in-standard-generators).

&nbsp;

## Building Custom Generators


You can build your own generator for type `T` by manually defining the conforming generator type `GenFunction<T>`. You can refer to [Building Custom Generators from Scratch](CustomGenerator.md) for more information.

While you can build a custom generator from scratch, it's usually not recommended as there is a better option - using a **generator combinator**. Generator combinators are toolkit for building new generators based on existing ones.
They can also be chained to create another generator out of themselves. See [Combinators](Combinators.md) page for the detail.

&nbsp;

[^generatorT]: In fact, `Arbitrary<T>` inherits from `Generator<T>`, which provides those helpers.
