# **cppproptest**

`cppproptest` is a property-based testing library for C++. It focuses on usability with following features included:

* Out-of-box [generators](Generators.md#arbitraries-provided-by-cppproptest) for primitives and standard containers

* Powerful [generator combinators](Combinators.md) for creating new generators from existing generators
* [Shrinking capability](Shrinking.md) for automated debugging support
* [Stateful testing support](StatefulTesting.md) for testing state changes
* [Concurrency testing support](ConcurrencyTesting.md) for testing concurrent state changes

You can get started with `cppproptest` on this [page](GettingStarted.md).

&nbsp;

# Why Property-based Testing?

## Generalization and Abstraction

Property-based testing, or PBT in short, lets you write a test using abstract idea, instead of introducing some dummy examples or contrived scenarios that are either too trivial or too complicated. You can write tests focusing on specification or requirements - which are essentially the combination of input domain and expected behavior of the tested component. As a result, we can say with higher confidence that the tested component works with general input domain.

### Clear Separation of Variants and Invariants

Describing a test using input domain and expected behavior means that we have clear cut between variants and invariants.
In reality, we often mix variants and invariants in our tests in typical *example-based tests*. This is one of the reasons why those tests become hard to maintain and difficult to read over time. Let's see following example-based test for an encoder and a decoder:

```cpp
// a text encoded and then decoded must be identical to original
MyEncoder encoder;
MyDecoder decoder;
auto encodedMsg = encoder.encode("Some dummy content that hopefully prove or disprove this works");
auto decodedMsg = decoder.decode(encodedMsg);
ASSERT_EQ("Some dummy content that hopefully prove or disprove this works", decodedMsg);
```

Even a simple example like this is confusing, as some dummy string value seems like as if it's playing a significant role in the test but in fact has no actual meaning. Turning this fixed dummy value to a free variable as a function parameter would lead to greater generalization of the test:

```cpp hl_lines="1 8"
[](std::string originalMsg) {
    // a text encoded and then decoded must be identical to originalMsg
    MyEncoder encoder;
    MyDecoder decoder;
    auto encodedMsg = encoder.encode(originalMsg);
    auto decodedMsg = decoder.decode(encodedMsg);
    PROP_ASSERT_EQ(originalMsg, decodedMsg);
};
```

The new free variable `originalMsg` becomes the variant part (input domain) of the test while the rest becomes the invariants (property of the tested components). In order to turn this into a concrete test run, we can feed in some random values for the free variable `originalMsg`.

This can be achieved by enclosing this function with `forAll()` function, which calls the given test function multiple times with some randomly generated values for the free variable `originalMsg`:

```cpp hl_lines="1"
forAll([](std::string originalMsg) {
    // a text encoded and then decoded must be identical to original
    MyEncoder encoder;
    MyDecoder decoder;
    auto encodedMsg = encoder.encode(originalMsg);
    auto decodedMsg = decoder.decode(encodedMsg);
    PROP_ASSERT_EQ(originalMsg, decodedMsg);
});
```

`forAll()` will test the encoder and decoder against arbitrary input strings, by calling the property function 200 times (the default number of runs) with random strings, instead of relying on a dummy value. This let's you find cases that can disprove the property by validating it with various forms of inputs.

With such property-based tests, we are clearly separating invariants from variants. Thus tests become more readable, get easier to maintain, and carry test writers' intention better.

## Convenience and Versatility

Property-based testing often provides with the convenient out-of-box generators and combinators (that lets you make new generators out of existing ones), and lets you to effortlessly define an input domain for your test. You can specify your input domain using powerful notions - range, constraint, union, transformation, composition, and dependency, just to name a few.

Following example shows how string input domain can be specified with certain constraints using combinators:

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

The framework automatically identifies the parameter types of given property function. This automation allows any number of parameters of simple or complex types can be used to define a property-based test:

```cpp
forAll([](int a, long b, float c, std::string d, std::vector<std::string> e, std::map<int, std::string> e) {
  // Do stuff with a, b, ..., and e
});
```

It then automatically feeds in the randomly generated values of those types to call the given function multiple times. With this powerful generation engine, we can fully parameterize and randomize our tests with high flexibility but with much less effort.

You don't need to care too much about *how* to test your requirements. Most of it is automatically done for you by the framework, letting you to focus on *what* to test.


## Reusability and Scalability

As it defines input domain with generators and combinators, Property-based testing lets you define and reuse existing input domain and properties to build new input domain and properties with ease. These new ones then can be reused as building blocks for next tests and so on.

Reusable nature of property-based testing lets you write tests with scale. Complex combinations can be readily tested if simpler pieces are already available by tests written previously.

&nbsp;


# Further Topics and Details:

* [Getting Started with cppproptest](GettingStarted.md)
* [Introduction to generators](Generators.md)
    * [Arbitraries](Generators.md#arbitraries---the-globally-default-generators)
    * [Built-in generators](Generators.md#arbitraries---the-globally-default-generators)
* [Generator combinators for creating generators from existing ones](Combinators.md)
* [Counterexamples and notion of shrinking for automated debugging](Shrinking.md)
* [Printing facilities](Printing.md)
* [Stateful testing with `cppproptest`](StatefulTesting.md)
* [Concurrency testing with `cppproptest`](ConcurrencyTesting.md)
* [Advanced mocking with `cppproptest` (work in progress)](Mocking.md)
