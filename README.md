# cppproptest

`cppproptest` is a property-based testing library for C++. It focuses on usability with following features included:

* Out-of-box generators for primitives and standard containers. See the [full list of built-in generators](doc/Generators.md#arbitraries-provided-by-cppproptest) for more.

* Versatile generator combinators for creating custom generators based on existing ones. See [generator combinators](doc/Combinators.md) for more.
* [Shrinking](doc/Shrinking.md) for automated debugging support
* [Stateful testing support](doc/StatefulTesting.md) for testing state changes
* [Concurrency testing support](doc/ConcurrencyTesting.md) for testing concurrent state changes

You can [get started with `cppproptest` in this page](doc/GettingStarted.md).

## Example: Turning conventional unit tests into property tests

A *property function* is in the form of `function<bool(InputType...)>` (or `function<void(InputType...)`)

```cpp
[](int a, int b) -> bool {
    return a + b == b + a;
}
```

A property-based testing library can generate random combinations of `a` and `b` and validate the given property function whether it always returns `true` for all the generated combinations (or, returns without exception for `void` returning property functions). 

A typical outcome of a property test would be:

> OK, passed 1000 tests

or 

> Falsifiable after 12 tests, where
>   a = 4,
>   b = -4

Among many other benefits, property-based tests can immediately replace dull dummy-based tests, such as:

```cpp
    // typical dummy-based test 
    // a text encoded and then decoded must be identical to original
    MyEncoder encoder;
    MyDecoder decoder;    
    auto original = "Hello world";
    auto encoded = encoder.encode(original);
    auto decoded = decoder.decode(encoded);
    ASSERT_EQ(original, decoded);

```

This can be turned into a property-based test, which fully tests the component againt arbitrary input strings:

```cpp
    // property test 
    forAll([](std::string original) {
        // a text encoded and then decoded must be identical to original
        MyEncoder encoder;
        MyDecoder decoder;    
        auto encoded = encoder.encode(original);
        auto decoded = decoder.decode(encoded);
        PROP_ASSERT_EQ(original, decoded);
    });
```

## Power of automation and versatility

```cpp
forAll([](int a, long b, float c, std::string d, std::vector<std::string> e, std::map<int, std::string> e) {
  // Do stuff with a, b, ..., and e
});
```

As seen in previous example, at the core of a property-based testing library, there is this `forAll` function. This itself is a powerful value generation engine. It identifies the parameter types of a given property function. It then automatically feeds in the random generated values of those types to the function. While you can use the out-of-box, default generators, you can build your own or fine-tune existing generators. Property-based testing libraries often accompany with versatile toolkit based on functional programming paradigm, to conveniently and precisely define new generators.

Given this powerful generation engine, we can fully parameterize and randomize our tests with high flexibility but little effort. You don't need to care much about *how* to test your requirements. Much of it is automatically done for you by the test library. With property-based testing, you can focus on two things: 

* Identifying requirements that your components should fulfill and writing them as test
* Defining the inputs to be tested (i.e. generators. Optional if you're using the defaults)

Here is the comparison table showing some of the benefits of writing property based tests over conventional unit tests:


|                   | Conventional Unit Tests   | **Property-Based Tests**     |
| ----------------- |---------------------------| ---------------------------- |
| Test inputs       | Dummy combinations        | Auto-generated combinations  |
| Test body         | Contrived scenarios       | Simple properties            |
| Test target       | Low-level implementations | High-level requirements      |
| Code Coverage     | Low                       | High                         |
| Readability       | Low                       | High                         |
| Confidence        | Low                       | High                         |
| Discovers new bugs| No                        | Yes                          |
| Debugging failures| Manual                    | Automated (via shrinking)    |


&nbsp;

## Further topics and details of the library can be found in:

* [Why property-based testing?](doc/WhyPropertyBasedTesting.md)
* [Getting Started with `cppproptest`](doc/GettingStarted.md)
* [Using and Defining Generators](doc/Generators.md)
* [Generator Combinators for Creating Generators from Existing Ones](doc/Combinators.md)
* [Counterexamples and Shrinking](doc/Shrinking.md)
* [Printing and debugging facilities](doc/Printing.md)
* [Stateful Testing with `cppproptest`](doc/StatefulTesting.md)
* [Concurrency Testing with `cppproptest`](doc/ConcurrencyTesting.md)
* [Advanced Mocking with `cppproptest`](doc/Mocking.md)

