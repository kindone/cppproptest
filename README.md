# cppproptest

`cppproptest` is a property-based testing framework for C++. 

## Introduction to Property-based testing

### An attempt to approximate software quality 

Property-based testing, or sometimes refered to as *specification-based testing*, tries to verify software's integrity by checking the outcomes of *properties* or *specifications* with massive input combinations. This can be viewed as an inductive way of checking software quality.

```cpp
    // check the property: 'content that has been encoded and then decoded should be identical to the original' 
    MyEncoder encoder;
    MyDecoder decoder;
    auto encoded = encoder.encode(original);
    auto decoded = decoder.decode(encoded);
    ASSERT_EQ(original, decoded);
```

Many property-based testing frameworks derive their ideas from [QuickCheck](https://en.wikipedia.org/wiki/QuickCheck), written in Haskell. Quickcheck's basic idea is to quickly prove a theorem, as the name suggests. You can define an abstract property (or 'specification') of a software component, and let the test framework prove or disprove that property by feeding in random (but valid) input combinations. We can see it as an attempt to approximate software quality. As the number of trials grows, more accurate the approximation it becomes.

### Hybrid of static and dynamic code analyses

Property-based testing approach can be considered somewhere in the middle of static analysis and dynamic analysis. Software integrity and defects can be validated in the form of properties or specifications in declarative fashion, as in static code analyses, but by actually running the code, as in dynamic code analyses. By actually running the code, we can check for many software quality requirements and issues that are usually not feasible to check with static code analyses. See the following comparison:

#### Static code analysis

* Analyzes the code in deductive fashion, examining every possible code path 
* Does not require to execute the code
  * Can find defects that are not in usual control paths
  * Might lacks precision of outcomes because actual execution context is not considered
* May raise false alarms
* Highly depends on strength of its deduction engine
  * Deducing for complex runtime scenarios is limited

#### Dynamic code analysis

* Checks the actual outcomes by executing the code
  * Can find complex runtime issues that are not implemented in static analyses
  * Might not cover edgy cases where the control paths are rarely taken
* Issue analyses often require huge effort from the tester 

#### Property-based testing

* Checks the actual outcome of properties by executing the code multiple times, with different input combinations
  * Attempts to inductively verify the requirements
  * Can find complex runtime issues
  * Also attemts to cover edgy cases with random generated inputs
* Issue analyses are semi-automated (See [shrinking](doc/Shrinking.md) for more)


&nbsp;

## An Example: Turning conventional unit tests into property tests

A property is in the form of function `(Input0, ... , InputN) -> bool` (or `(Input0, ... , InputN) -> void`)

```cpp
[](int a, int b) -> bool {
    return a + b == b + a;
}
```

A property-based testing framework attempts to generate combinations of `a` and `b` and validate the function whether it always returns `true` for all the combinations. 

> OK, passed 1000 tests

or 

> Falsifiable after 12 tests, where
>   a = 4,
>   b = -4 **(this failure is not actual and only exemplary)**

Among many other benefits, property-based tests can immediately replace dull dummy-based tests, such as:

```cpp
// typical dummy-based test 
TEST(Suite, test) {
    // a text encoded and then decoded must be identical to original
    MyEncoder encoder;
    MyDecoder decoder;    
    auto original = "Hello world";
    auto encoded = encoder.encode(original);
    auto decoded = decoder.decode(encoded);
    ASSERT_EQ(original, decoded);
}
```

This can be turned into a property-based test, which fully tests the component againt arbitrary input strings:

```cpp
// property test 
TEST(Suite, test) {
    ASSERT_TRUE(forAll([](std::string original) {
        // a text encoded and then decoded must be identical to original
        MyEncoder encoder;
        MyDecoder decoder;    
        auto encoded = encoder.encode(original);
        auto decoded = decoder.decode(encoded);
        PROP_ASSERT_EQ(original, decoded);
    }));
}
```

Here is the comparison table showing some of the benefits of turning conventional unit tests into property based tests:


|                   | Conventional Unit Tests   | **Property-Based Tests**     |
| ----------------- |---------------------------| ---------------------------- |
| Style             | Procedural/concrete       | Functional/abstract/declarative|
| Test inputs       | Example values            | Auto-generated combinations  |
| Finds new bugs    | No                        | Yes                          |
| Code Coverage     | -                         | Higher                       |
| Readability       | -                         | Higher                       |
| Debugging failures| -                         | Automated (shrinking)        |
| Stateful tests    | -                         | Supported                    |
| Concurrency tests | -                         | Supported                    |

&nbsp;

## Why property-based testing for C++?

Property-based tesing relies on massive input combinations to be attempted to achieve high confidence. 
For example, if you have a function with 10 boolean flags to be thoroughly tested in runtime, you need to test it with `2`<sup>`10`</sup> ` = 1024` combinations. This kind of complexity has been often considered as unmanageable and something that should be avoided. 

However, as most C++ unit components (as single unit or multiple components combined) tend to execute blazingly fast on modern machines, running them 1000 times is usually not an issue. Unless it's involving external devices like secondary disk or network communication, running a typical C++ component 1000 times would normally end up finishing under 1 second barrier. This is why C++ and property-based testing is a perfect pair.


&nbsp;

## Further topics and details of the framework can be found in:

* [Getting Started with `cppproptest`](doc/GettingStarted.md)
* [Using and Defining Generators](doc/Generators.md)
* [Counter Examples and Shrinking](doc/Shrinking.md)
* [Stateful Testing with `cppproptest`](doc/StatefulTesting.md)
* [Concurrency Testing with `cppproptest`](doc/ConcurrencyTesting.md)
* [Advanced Mocking with `cppproptest`](doc/Mocking.md)

