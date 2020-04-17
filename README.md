# proptest
Property-Based Testing Framework for C++

## Property and Check

`property` defines a property with optional configuration and `check` is a shorthand for`property(...).check()`. `Property::check` performs property-based test using given callable (a function, a functor, or a lambda).


```cpp
check([](int a, int b) -> bool {
    PROP_ASSERT(a+b == b+a);
    PROP_STAT(a+b > 0);
    return true;
});
```

is equivalent to 

```cpp
property([](int a, int b) -> bool {
    PROP_ASSERT(a+b == b+a);
    PROP_STAT(a+b > 0);
    return true;
}).check();
```

### Defining property

## Generators

### Primitives

### Combinators

## Shrinking

## Stateful testing

## Concurrency testing

## Generating Mocks
## 
