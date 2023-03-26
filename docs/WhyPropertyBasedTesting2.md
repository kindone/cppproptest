
## An attempt to approximate software quality

Property-based testing, or sometimes refered to as *specification-based testing*, tries to verify software's integrity by validating the requirements of a software component, where the requirements are often written as *properties*(or *specifications*). They are validated using massive input combinations.

```cpp
    // encode & decode are inverse functions
    ASSERT(decode(encode(text)) == text);
```

Many property-based testing implementations derive their ideas from [QuickCheck](https://en.wikipedia.org/wiki/QuickCheck), which was originally written in Haskell. Quickcheck's basic idea is to *quickly prove a theorem*, as the name suggests. But how can anything possibly be *proved* about software? A software piece is often too complex to reason about in definitive way. If we randomly choose 100 input combinations to test your component and confirm that it satisfies the required properties with every 100 of the combinations, you may argue that it's still not *proving* anything. But how about if we increase the number of combinations to 1,000, or 10,000?

Certainly there still remains some possibility that one of the untested input combinations might fail the properties. But we can say this as an approximation - an approximation of software integrity or quality. As the number of evidences grows, more accurate the approximation it becomes. It's often very effective to approximate an answer when it cannot be easily obtained. It's sometimes the only feasible way.

Property-based testing can do exactly that. You define a property (or 'specification') of a software component should satisfy, then you can let the test library *prove* or *disprove* that requirement automatically by feeding in random (but valid) input combinations.

In fact, often the underlying issues of a component are easily detected than you can imagine. Some properties may hold true with a few combinations, but such *luck* usually cannot last for long. The issues reveal themselves eventually.

In property-based testing, software requirements can be validated in automated fashion as in static code analyses, but by actually running the code as in dynamic code analyses. By actually running the code, we can check for many software requirements and issues that are usually not feasible to find with static code analyses.

&nbsp;


## Why property-based testing for C++?

Property-based tesing relies on massive input combinations to be attempted to achieve high confidence.
For example, if you have a function with 10 boolean flags to be thoroughly tested in runtime, you need to test it with `2`<sup>`10`</sup> ` = 1024` combinations. This kind of complexity has been often considered as unmanageable and something that should be avoided. However, as most C++ unit components (as single unit or multiple components combined) tend to execute blazingly fast on modern machines, running them 1000 times is usually not an issue. Unless it's involving external devices like secondary disk or network communication, running a typical C++ component 1000 times would normally end up finishing under a second barrier. This is why C++ and property-based testing is a fine pair.

I feel many of the existing property-based testing implementation in C++ are either outdated or too daunting to learn due to their complexity.
`cppproptest` focuses on lowering the learning curve while maintaining feature-completeness.
