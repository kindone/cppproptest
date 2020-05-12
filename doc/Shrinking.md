## Shrinking


Compared to [generators](Generators.md), shrinking is a relatively optional feature of property-based testing. But it plays very important role in terms of usability and debugging. 

> Shrinking is the process of finding a failing combination of arguments with simpler representation.

Shrinking shines when a property fails and you want to find out why - starting with whether it was the component that has a bug or the test that was badly designed. 

```cpp
forAll([](int a, int b, bool c, std::vector<int> d) -> bool {
    PROP_ASSERT(...);
});
```

If above property fails, you'd immediately want to know which combination of the arguments `a`, `b`, `c`, and `d` is responsible for the failure. In some cases, a boolean flag such as `c` in this example could be the root cause. If `c` is only responsible for the failure, we can keep other arguments `a`, `b`, and `d` as simple as possible. Find such combination of arguments is what we do 

Shrinking can do this automatically. When a failure is encountered, `forAll` performs *shrinking* of the failing arguments. In other words, it tries to find another input combination that still fails the property, but with a simpler representation.
If an integer is big, take smaller number. If a boolean is `true`, then take `false`. If a vector has many elements, remove some of the elements, and so on.

There is no universal definition of *simple*, but we can set a rule of thumb:

* Boolean: `false` (turning flag off) is simpler than `true`
* Numeric:
    * remove sign or take a smaller absolute value/exponent
    * round some of the digits
* String: drop some of characters in the back
* Containers: remove some of the elements (in the back)
