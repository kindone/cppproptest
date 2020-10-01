# cppproptest

`cppproptest` is a property-based testing framework for C++. 

## Why Property-based testing?

### An attempt to approximate software quality 

Property-based testing, or sometimes refered to as *specification-based testing*, tries to verify software's integrity by validating the requirements of a software component, where the requirements are written as *properties* or *specifications*. They are validated using massive input combinations. This can be viewed as an inductive or approximate way of checking software quality.

```cpp
    // Encoder & Decoder are inverse functions: text encoded and then decoded must equal to original text
    ASSERT(decode(encode(text)) == text);
```

Many property-based testing implementations derive their ideas from [QuickCheck](https://en.wikipedia.org/wiki/QuickCheck), written in Haskell. Quickcheck's basic idea is to *quickly prove a theorem*, as the name suggests. But how can anything possibly be *proved* about software? A software piece is often too complex to reason about in definitive way. If we randomly choose 100 input combinations to test your component and confirm it satisfies the required properties with every 100 of the combinations, you can argue it's still not *proving* anything. How about if we increase the number to 1000, or 10000? Certainly there still remains some possibility that one of the untested input combinations might fail the properties. But we can say this as an approximation - an approximation of software integrity or quality. As the number of evidences grows, more accurate the approximation it becomes. It's often very effective to approximate an answer when it cannot be easily obtained. It's sometimes the only feasible way. Property-based testing can do exactly that. You define a property (or 'specification') of a software component should satisfy, then you can let the test framework *prove* or *disprove* that property by feeding in random (but valid) input combinations.  

If the goal is to find a bug in a software, or in other words, to disprove the requirements, then this approach can be very effective in those cases. Why? Because it's often easier to prove that there is some bug in a software than to prove that it does not.  

### Harnessing the power of automation

Property-based testing approach can be considered to be somewhere in the middle of static analysis and dynamic analysis. Software requirements can be validated in automated fashion as in static code analyses, but by actually running the code as in dynamic code analyses. By actually running the code, we can check for many software quality requirements and issues that are usually not feasible to check with static code analyses. See the following comparison:

#### Static code analysis

* Analyzes the code with deduction, examining every possible code path 
* Fully automated
* Does not actually execute the code
  * Can find defects that are not in usual control paths
  * May lack precision in the outcomes because actual software context is not considered
* Cannot deal with software-specific requirements
* May raise false alarms
* Highly depends on strength of its deduction engine
  * Deducing against complex scenarios is limited

#### Dynamic code analysis

* Checks the actual outcomes by executing the code
  * Can find complex runtime issues that are not implemented (or able to implement) in static analyses
  * Might not cover edgy cases where the control paths are rarely taken
* Usually requires user to manually define how to test the requirements
* Analysing an issue often require huge effort from the tester

#### Property-based testing

* Requirements are described as properties
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

