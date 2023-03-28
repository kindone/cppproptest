# Simplifying Failed Inputs

Shrinking is powerful notion widely utilized in most property-based testing libraries. It automates some of typical debugging process and greatly reduces the effort required to analyse a test failure. `cppproptest` fully supports shrinking for all built-in generators and combinators.

## Counterexample and Shrinking

Compared to [generators](Generators.md), shrinking is a relatively optional feature of property-based testing. But it plays very important role in terms of usability and debugging. Here is an example:

```cpp
forAll([](int a, double b, bool c) -> bool {
    PROP_ASSERT(...);
});
```

When a property fails during a property-based test, a counterexample is found.

> Falsifiable after 12 tests, where a = -13680124, b = 7524.68454e-14, c = true

Here, `a`, `b`, and `c`'s tuple `(-13680124, 7524.68454e-14, true)` is a counterexample that disproves a property. You can reproduce the property failure with this input combination so that you can further reason about its root cause.

You would immediately want to know which combination of the arguments `a`, `b`, and `c` is responsible for the failure. In some cases, a boolean flag such as `c` in this example could have triggered the failure. If `c` is the only responsible for the failure, we can pull other arguments `a` and `b` out of the equation and keep them as simple as possible. Finding such relationship of arguments is what we would typically do to indentify the root cause of the failure.

*Shrinking* exactly automates this process. When a failure is encountered, `forAll` *shrinks* the failing input combination. In other words, it tries to find another input combination that still fails the property, but with simpler representation.
If an integer is big, it tries with a smaller number. If a boolean is `true`, then tries with `false`. If a vector has many elements, it tries with removing some of the elements, and so on. If possible, it will come up with much more meaningful set of arguments that led to the failure. In this specific example, `a`, `b`, and `c` tuple can be shrunk from the complicated `(-13680124, 7524.68454e-14, true)` to much simpler `(0, 0, true)`, because `a` and `b` has no role in the failure.

In short,

> *Shrinking* is the process of finding another failing combination of arguments with simpler representation.

Although There is no universal definition of *simple* or *simpler*, we can agree on some basic ideas:

* Booleans: `false` (turning a flag off) is simpler than `true` (turning on)
* Numerics:
    * remove sign or take a smaller absolute value/exponent (e.g. `-34` -> `34`, `16384` -> `1024`, `12e55` -> `12e20`)
    * round some of the digits (e.g. `-29.5134` -> `-29`)
* Strings: drop some of characters in the back (e.g. `"Hello world!"` -> `"Hello"`)
* Containers: remove some of the elements (in the back)
    * `[0,1,2,3,4,5]` -> `[0,1,2]`

*Shrinking* also shines during the early stage of writing a test. It can quickly summarize a test failure so that we can easily identify whether it was the component that had a bug or the test itself that was badly designed.
