# proptest

`proptest` is a property-based testing framework for C++. 

## Property-based testing

Many property-based testing frameworks derive from [QuickCheck](https://en.wikipedia.org/wiki/QuickCheck) in Haskell. 
Its basic idea is to quickly prove a theorem, as the name 'QuickCheck' suggests. 
You can define an abstract property of a component, and let the test framework prove or disprove that property by feeding in massive input combinations. 

This approach is often said to be somewhere in the middle of static analysis and dynamic analysis. Software integrity and defects can be validated in definitive fashion, as in static code analysis, but by actually running the code, as in dynamic code analysis.

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
    check([](std::string original) {
        // a text encoded and then decoded must be identical to original
        MyEncoder encoder;
        MyDecoder decoder;    
        auto encoded = encoder.encode(original);
        auto decoded = decoder.decode(encoded);
        PROP_ASSERT_EQ(original, decoded);
    });
}
```

Here are some of the benefits of turning conventional unit tests into property based tests:


|                   | Conventional Unit Tests   | **Property-Based Tests**     |
| ----------------- |---------------------------| ---------------------------- |
| Paradigm          | Procedural                | Functional                   |
| Test inputs       | Dummy values (with bias)  | Auto-generated combinations  |
| Style             | Concrete, imperative      | Abstract, declarative        |
| Finding bugs      | Less likely               | More likely                  |
| Code Coverage     | Low                       | High                         |
| Readability       | Low                       | High                         |
| Extensibility     |                           |                              |
| &emsp; of params  | No                        | Yes                          |
| &emsp; of mocks   | No                        | Yes                          |
| Encourages change | Yes                       | Yes, even more               |
| Input generation  | Manual                    | Automatic                    |
| Debugging failures| Manual                    | Backed by automatic shrinking|
| Stateful tests    | Manual                    | Semi-automatic               |
| Concurrency tests | Manual                    | Semi-automatic               |
| Developer efforts | More                      | Less                         |

&nbsp;

## Further topics and details of the framework can be found in:

* [Getting Started](doc/GettingStarted.md)
* [Using and Defining Generators](doc/Generators.md)
* [Counter Examples and Shrinking](doc/Shrinking.md)
* [Stateful Testing with Property-based Testing Framework](doc/StatefulTesting.md)
* [Concurrency Testing with Property-based Testing Framework](doc/ConcurrencyTesting.md)
* [Advanced Mocking with Property-based Testing Framework](doc/Mocking.md)

