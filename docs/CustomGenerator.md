
# Building Custom Generator from Scratch

## `GenFunction<T>` - Common representation for all generators for type `T`

All generators, including the default ones, share the same base *function* type. A generator can be a callable (function, functor, or lambda) with following common signature:

```cpp
// (Random&) -> Shrinkable<T>
```

This can be represented as (or coerced to) a standard function type, `std::function<Shrinkable<T>(Random&)>`. In `cppproptest`, this function type is aliased as `GenFunction<T>`. We will use this term *GenFunction* throughout this page to refer the generator function type.

```cpp
template <typename T>
using GenFunction = std::function<Shrinkable<T>(Random&);
```

By the way, you may have noticed a strange template type `Shrinkable` in this signature. You can refer to [`Shrinkable`](Shrinking.md) for its further detail, but it can be treated as a wrapper for type `T` for now. So a generator (`Generator<T>`) basically generates a value of type `T` from a random number generator of `Random` type. A generator can be defined as function, functor, or lambda, as following:

```cpp
// lambda style
auto myIntGen = [](Random& rand) {
    int smallInt = rand.getRandomInt8();
    return make_shrinkable<int>(smallInt);
};

// function style
Shrinkable<int> myIntGen(Random& rand) {
    int smallInt = rand.getRandomInt8();
    return make_shrinkable<int>(smallInt);
}

// functor style
struct MyIntGen {
    Shrinkable<int> operator()(Random& rand) {
        int smallInt = rand.getRandomInt8();
        return make_shrinkable<int>(smallInt);
    }
};
```

## `Generator<T>` - Decorator class for supercharging a generator

The template class `Generator<T>` is an abstract functor class that also coerces to `GenFunction<T>`. A `Generator<T>` gives access to some useful methods so that you can wrap your callable with this to decorate with those methods. As all accompanied generators and combinators of `cppproptest` produce decorated `Generator<T>`s, you can use the utility methods out-of-box.

```cpp
// decorate a GenFunction with Generator<T>
auto myIntGen = Generator<int>([](Random& rand) {
    int smallInt = rand.getRandomInt8();
    return make_shrinkable<int>(smallInt);
});

// .filter and other utility methods can be used once the generator is decorated with Generator<T>
auto evenGen = myIntGen.filter([](int& value) {
    return value % 2 == 0;
}); // generates even numbers only
```
