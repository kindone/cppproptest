# cppproptest

`cppproptest` is a property-based testing library for C++. It focuses on usability with following features included:

* Out-of-box generators for primitives and standard containers. See the [full list of built-in generators](doc/Generators.md#arbitraries-provided-by-cppproptest) for more.

* Versatile generator combinators for creating custom generators based on existing generators. See [generator combinators](doc/Combinators.md) for more.
* [Shrinking](doc/Shrinking.md) for automated debugging support
* [Stateful testing support](doc/StatefulTesting.md) for testing state changes
* [Concurrency testing support](doc/ConcurrencyTesting.md) for testing concurrent state changes

You can get started with `cppproptest` on this [page](doc/GettingStarted.md).

&nbsp;

# Why Property-based testing?

## Generalization and abstraction

Property-based testing, or PBT in short, lets you write a test using abstract idea, instead of some dummy examples or contrived scenarios that are easy to break over time and hard to interpret. You can write tests using specification or requirements - which are essentially expressed as combination of input domain (=generators) and expected behavior (=properties) of tested component. 

Property-based tests can immediately replace example-based tests, such as:

```cpp
MyEncoder encoder;
MyDecoder decoder;    
auto original = "Some dummy content that hopefully prove or disprove this encoder/decoder works";
auto encoded = encoder.encode(original);
auto decoded = decoder.decode(encoded);
ASSERT_EQ(original, decoded);
```

This can be turned into a property-based test, which fully tests the components against arbitrary input strings:

```cpp
forAll([](std::string original) {
    // a text encoded and then decoded must be identical to original
    MyEncoder encoder;
    MyDecoder decoder;    
    auto encoded = encoder.encode(original);
    auto decoded = decoder.decode(encoded);
    PROP_ASSERT_EQ(original, decoded);
});
```

Example inputs were turned into fully randomized inputs. As a result, we can say with higher confidence that the tested component works with more general input domain.

## Clear separation of variants and invariants

Describing a test using input domain and expected behavior means that we have clear cut between variants and invariants. 
We often mix variants and invariants in our tests in typical *example-based tests*. This is one of the reasons why those tests become hard to maintain and difficult to read over time. With property-based tests, however, we can clearly separate invariants as properties and variants as input domain. Thus tests become more readable, gets easier to maintain, and carry test writers' intention better.

## Convenience and versatility

Property-based testing often provides with the convenient out-of-box generators and combinators (that lets you make new generators out of existing ones), and lets you to effortlessly define the input domain for your tests. You can specify your input domains using various notions - range, constraint, union, transformation, composition, and dependency, just to name a few.

```cpp
// a tailored string generator
auto stringGen = Arbitrary<int>()
    .filter([] (int& num) { num % 2 == 0; }) // even numbers only
    .map([] (int& num) {
        return "<" + std::to_string(numStr) + ">"; // string like "<0>", ..., "<n>"
    });
    
// property
forAll([](std::string original) {
    // ... //
}, stringGen);
```

The framework identifies the parameter types of a given property function. It then automatically feeds in the randomly generated values of those types to the function. Any number of parameters of simple or complex types can be used to test a property:

```cpp
forAll([](int a, long b, float c, std::string d, std::vector<std::string> e, std::map<int, std::string> e) {
  // Do stuff with a, b, ..., and e
});
```

Given this powerful generation engine, we can fully parameterize and randomize our tests with high flexibility but little effort. You don't need to care too much about *how* to test your requirements. Much of it is automatically done for you by the framework.


## Reusability and Scalability

As it defines input domain with generators and combinators, Property-based testing lets you define and reuse existing input domain and properties to build new input domain and properties with ease. These new ones then can be reused as building blocks for next tests and so on. Reusable nature of property-based testing lets you write tests with scale. Complex combinations of components can be readily tested if simpler pieces are already available by tests written previously.

&nbsp;


# Further topics and details:

* [Introduction to generators](doc/Generators.md)
    * [Arbitraries](doc/Generators.md#arbitraries---the-globally-default-generators)
    * [Built-in generators](doc/Generators.md#arbitraries---the-globally-default-generators)
* [Generator combinators for creating generators from existing ones](doc/Combinators.md)
* [Counterexamples and notion of shrinking for automated debugging](doc/Shrinking.md)
* [Printing facilities](doc/Printing.md)
* [Stateful testing with `cppproptest`](doc/StatefulTesting.md)
* [Concurrency testing with `cppproptest`](doc/ConcurrencyTesting.md)
* [Advanced mocking with `cppproptest` (work in progress)](doc/Mocking.md)

