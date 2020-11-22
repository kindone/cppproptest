# Using and Defining Generators 

Generators play key role in property-based testing and `cppproptest`. You use _generators_ to generate randomized arguments for properties. A simple `forAll` statement utilizes generators under the hood.

```cpp
forAll([](int age, std::string name) {
});
```

This `forAll` takes a function having parameters of types `int` and `std::string`. The parameter types are first extracted and then used to invoke the default generators for the types. Default generators are called *arbitraries* and above code is actually equivalent to the following:

```cpp
forAll([](int age, std::string name) {
}, Arbitrary<int>(), Arbitrary<std::string>());
``` 

Now you see that `forAll` actually requires some information how to generate the values of the extracted types. They're just hidden as defaults.

## `GenFunction<T>`

All generators, including the default ones, share the same base *function* type. A generator can be a callable (function, functor, or lambda) with following common signature:

```cpp
// (Random&) -> Shrinkable<T>
```

This can be represented as (or coerced to) a standard function type, `std::function<Shrinkable<T>(Random&)>`. In `cppproptest`, this function type is aliased as `GenFunction<T>`. We will use this term *GenFunction* throughout this page to refer the generator function type.

```cpp
template <typename T>
using GenFunction = std::function<Shrinkable<T>(Random&);
```

By the way, you may have noticed a strange template type `Shrinkable` in this signature. You can refer to [`Shrinkable`](doc/Shrinking.md) for its further detail, but it can be treated as a wrapper for type `T` for now. So a generator (`Generator<T>`) basically generates a value of type `T` from a random number generator of `Random` type. A generator can be defined as function, functor, or lambda, as following: 

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

## `Generator<T>`

The template class `Generator<T>` is an abstract functor class that also coerces to `GenFunction<T>`. A `Generator<T>` gives access to some useful methods so that you can wrap your callable with this to decorate with those methods. As all accompanied generators and combinators of `cppproptest` produce decorated `Generator<T>`s, you can use the utility methods with ease.

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


## Arbitraries

An `Arbitrary<T>` or its alias `Arbi<T>` is a generator type that also coerces to `GenFunction<T>`.
These generator types are specially treated in `cppproptest`. An arbitrary serves as globally defined default _generator_ for the type. If a default generator for a type is available, `cppproptest` uses that generator to generate a value of that type, if no custom generator is provided.

```cpp
// if there is no default generator available, you should provide a generator for the type. 
forAll([](SomeNewType x) {
}, someNewTypeGen);


// if there is a default generator (Arbitrary<SomeNewType>) available, you may use that generator by omitting the argument 
forAll([](SomeNewType x) {
});
```

With template specialization, new `Arbi<T>` (or its alias `Arbitrary<T>`) for type `T` can be defined, if it isn't already defined yet. By defining an _Arbitrary_, you are effectively adding a default generator for a type to the library.

Following shows an example of defining an _Arbitrary_. Note that it should be defined under `proptest` namespace in order to be noticed and accessible in the library core.

```cpp
namespace proptest { // you should define your Arbi<T> inside the namespace

struct Arbi<Car> : ArbiBase<Car> {
  Shrinkable<Car> operator()(Random& rand) {
    bool isAutomatic = rand.getRandomBool();
    return make_shrinkable<Car>(isAutomatic); // make_shrinkable creates a Car object by calling Car's constructor with 1 boolean parameter
  }
};

}
```

As an `Arbitrary<T>` is also a `Generator<T>`, an arbitrary provides useful helpers for creating new generators from existing ones. `filter` is such a helper. It restrictively generates values that satisfy a criteria function. Following is an even number generator from the integer `Arbitrary`.

```cpp
// generates any integers
auto anyIntGen = Arbi<int>();
// generates even integers
auto evenGen = anyIntGen.filter([](int& num) {
    return num % 2 == 0;
});
```

You can find the full list of such helpers in section [Utility methods in standard generators](#Utility-methods-in-standard-generators).

&nbsp;

## Built-in generators

Built-in generators are in the form of Arbitraries. `cppproptest` provides a set of Arbitraries for immediate generation of types that are often used in practice.

* Boolean type:`bool`
* Character type: `char`
* Integral types: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
* Floating point types: `float`, `double`
* String types: 
        * `std::string` (defaults to generate ASCII character strings in \[0x01, 0x7F\] range)
	* `UTF8String` (a class which extends `std::string` and can be used to generate valid [UTF-8](https://en.wikipedia.org/wiki/UTF-8) strings by using `Arbi<UTF8String>`)
	* `CESU8String` (similar to UTF-8, but can be used to generate valid [CESU-8](https://en.wikipedia.org/wiki/CESU-8) strings)
	* `UTF16BEString` and `UTF16LEString` for [UTF-16](https://en.wikipedia.org/wiki/UTF-16) big and little endian strings. CESU-8 and Unicode types produce full unicode code point range of \[0x1, 0x10FFFF\], excluding forbidden surrogate code points (\[0xD800, 0xDFFF\])
* Shared pointers: `std::shared_ptr<T>` where an `Arbi<T>` or a custom generator for `T` is available. It's also useful for generating polymorphic types.
	```cpp
	struct Action {
	    virtual int get() = 0;
	};
	struct Insert : Action {
	    virtual void get() { return 1; }
	};
	struct Delete : Action {
	    virtual void get() { return 2; }	
	};
	Generator<std::shared_ptr<Action>>(...); // can hold both Insert and Delete
	```
* Standard containers: `std::vector`, `std::list`, `std::set`, `std::pair`, `std::tuple`, `std::map`
	* Arbitraries for containers can optionally take a generator for their element types
		```cpp
		// You can supply a specific generator for integers
		auto vecInt0to100 = Arbi<std::vector<int>>(interval<int>(0,100));
		// otherwise, Arbi<int> is used
		auto vecInt = Arbi<std::vector<int>>();
	        auto uppercaseGen = Arbi<std::string>(interval('A', 'Z'));
		```

	* `Arbi<std::Map>` provides setter methods for assigning key and value generators

		```cpp
		auto mapGen = Arbi<std::map<int,int>>();
		mapGen.setKeyGen(interval<int>(0,100)); // key ranges from 0 to 100
		mapGen.setElemGen(interval<int>(-100, 100)); // value ranges from -100 to 100
		```

   	* Containers provide methods for configuring desired sizes
		* `setMinSize(size)`, `setMaxSize(size)` for restricting to specific range of sizes
		* `setSize(size)` for restricting to specific size

		```cpp
		auto vecInt = Arbi<std::vector<int>>();
		vecInt.setSize(10); // 1) generated vector will always have size 10
		vecInt.setMinSize(1); // 2) generated vector will have size >= 1
		vecInt.setMaxSize(10); // generated vector will have size <= 10
                vecInt.setSize(1, 10); // 3) generated vector will have size >= 1 and size <= 10
		```

## Build Custom Generators with Generator Combinators

While you can build your own generator manually defining a `GenFunction<T>`, it's usually not recommended as there is a better option - generator combinators.
Generator combinators are the toolkit for building new generators based on existing ones. 
They can be chained to create another generator out of themselves. See [Combinators](./Combinators.md) page for the detail. 
