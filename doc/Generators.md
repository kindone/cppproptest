# Using and Defining Generators 

You can use generators to generate randomized arguments for properties.

A generator is a callable (function, functor, or lambda) with following signature:

```cpp
(Random&) -> Shrinkable<T>
```

You can refer to [`Shrinkable`](doc/Shrinking.md) for its further detail, but you can basically treat it as a wrapper for a value of type `T` here. So a generator generates a value of type `T` from a random generator. A generator can be defined as functor or lambda, as you would prefer.  

```cpp
auto myIntGen = [](Random& rand) {
    int smallInt = rand.getRandomInt8();
    return make_shrinkable<int>(smallInt);
};
```

## Arbitraries

An `Arbitrary` refers to default generator for a type. You can additionaly define an `Arbitrary<T>` for your type `T`, if it isn't already defined. By defining an `Arbitrary`, you can omit the custom generator argument that is needed to be passed everytime you defined a property for that type. Following shows an example for defining an `Arbitrary`. Note that it should be defined under `PropertyBasedTesting` namespace in order to be accessible in the framework.

```cpp
namespace PropertyBasedTesting {

struct Arbitrary<Car> : Gen<Car> {
  Shrinkable<Car> operator()(Random& rand) {
    bool isAutomatic = rand.getRandomBool();
    return make_shrinkable<Car>(isAutomatic);
  }
};

}
```

There are useful helpers for creating new generators from existing ones. 

`suchThat` is such a helper. It selectively generates values that satisfies a criteria function. Following is an even number generator from the integer `Arbitrary`.

```cpp
auto anyIntGen = Arbitrary<int>();
// generates even numbers
auto evenGen = suchThat<int>(anyIntGen, [](const int& num) {
    return num % 2 == 0;
});
```

You can find the full list of such helpers in section **Generator Combinators** below.

&nbsp;

## Arbitraries provided by `cppproptest`

Built-in generators are called Arbitraries. `cppproptest` provides a set of Arbitraries for immediate generation of types that are often used.

* `char` and `bool`
* Integral types: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
* Floating point types: `float`, `double`
* String types: `std::string` (defaults to generate ASCII character strings in \[0x01, 0x7F\] range), `UTF8String` (a class which extends `std::string` and can be used to generate valid [UTF-8](https://en.wikipedia.org/wiki/UTF-8) strings by using `Arbitrary<UTF8String>`), `CESU8String` (similar to UTF-8, but can be used to generate valid [CESU-8](https://en.wikipedia.org/wiki/CESU-8) strings), `UTF16BEString` and `UTF16LEString` for [UTF-16](https://en.wikipedia.org/wiki/UTF-16) big and little endian strings. CESU-8 and Unicode types produce full unicode code point range of \[0x1, 0x10FFFF\], excluding forbidden surrogate code points (\[0xD800, 0xDFFF\])
* Shared pointers: `std::shared_ptr<T>` where an `Arbitrary<T>` or a custom generator for `T` is available. It's useful for containing polymorphic types.
* Standard containers: `std::vector`, `std::list`, `std::set`, `std::pair`, `std::tuple`, `std::map`
	* Arbitraries for containers can optionally take a generator for their element types
		```cpp
		// You can supply a specific generator for integers
		auto vecInt0to100 = Arbitrary<std::vector<int>>(inRange<int>(0,100));
		// otherwise, Arbitrary<int> is used
		auto vecInt = Arbitrary<std::vector<int>>();
		```
	
	* `Arbitrary<std::Map>` provides setter methods for assigning key and value generators
		
		```cpp
		auto mapGen = Arbitrary<std::map<int,int>>();
		mapGen.setKeyGen(inRange<int>(0,100)); // key ranges from 0 to 100
		mapGen.setElemGen(inRange<int>(-100, 100)); // value ranges from -100 to 100
		```
	
   	* Containers provide methods for configuring desired sizes
		* `setMinSize(size)`, `setMaxSize(size)` for restricting to specific range of sizes
		* `setSize(size)` for restricting to specific size 
		
		```cpp
		auto vecInt = Arbitrary<std::vector<int>>();
		vecInt.setSize(10); // generated vector will always have size 10
		vecInt.setMinSize(1); // generated vector will have size greater than or equal to 1
		vecInt.setMaxSize(10); // generated vector will have size less than or equal to 10
		```

## Generator Combinators

Generator combinators are provided for building a new generator based on existing ones. They can be chained as they receive existing generator(s) as argument and returns new generator.

* `just<T>(T*)` or `just<T>(function<T()>)`: always generates specific value

### Pair and Tuples

* `pair<T1, T2>(gen1, gen2)` : generates a `std::pair<T1,T2>` based on result of generators `gen1` and `gen2`

	```cpp
	auto pairGen = pair<int, std::string>(Arbitrary<int>(), Arbitrary<std::string>());
	```

* `tuple<T1, ..., TN>(gen1, ..., genN)`: generates a `std::tuple<T1,...,TN>` based on result of generators `gen1` through `genN`

	```cpp
	auto tupleGen = tuple<int, std::string, double>(Arbitrary<int>(), Arbitrary<std::string>(), Arbitrary<double>());
	```

### Constructing an object

* `construct<T, ARG1, ..., ARGN>([gen1, ..., genM])`: generates an object of type `T` by calling its constructor that matches the signature `(ARG1, ..., ARGN)`. Custom generators `gen1`,..., `genM` can be supplied for generating arguments. If `M < N`, then rest of the arguments are generated with `Arbitrary`s.

	```cpp
	struct Coordinate {
	    Coordinate(int x, int y) {
		// ...
	    }
	};
	// ...
	auto coordinateGen1 = construct<Coordinate, int, int>(inRange(-10, 10), inRange(-20, 20));
	auto coordinateGen2 = construct<Coordinate, int, int>(inRange(-10, 10)); // y is generated with Arbitrary<int>
	```

### Applying constraints

* `suchThat<T>` or `filter<T>(gen, condition_predicate)`:  generates a type `T` that satisfies condition predicate (`condition_predicate` returns `true`)

	```cpp
	// generates even numbers
	auto evenGen = filter<int>(Arbitrary<int>(),[](const int& num) {
	    return num % 2 == 0;
	});
	```

### Transformation or mapping

* `transform<T,U>(gen, transformer)`: generates type `U` based on generator for type `T`, using `transformer` that transforms a value of type `T` to type `U`

	```cpp
	// generates string from integers (e.g. "0", "1", ... , "-16384")
	auto numStringGen = transform<int, std::string>(Arbitrary<int>(),[](const int& num) {
	    return std::string(num);
	});
	```

### Selecting from generators

* `oneOf<T>(gen1, ..., genN)`: generates a type `T` from multiple generators for type `T`, by choosing one of the generators randomly

	```cpp
	// generates a numeric within ranges (0,10), (100, 1000), (10000, 100000)
	auto evenGen = oneOf<int>(inRange<int>(0, 10), inRange<int>(100, 1000), inRange<int>(10000, 100000));
	```
	
	* `oneOf` can receive optional probabilitistic weights (`0 < weight < 1`, sum of weights must not exceed 1.0) for generators. If weight is unspecified for a generator, it is calculated automatically so that remaining probability among unspecified generators is evenly distributed.
	
	```cpp
	// generates a numeric within ranges (0,10), (100, 1000), (10000, 100000)
	auto evenGen = oneOf<int>(weighted(inRange<int>(0, 10), 0.8), weighted(inRange<int>(100, 1000), 0.15), inRange<int>(10000, 100000)/* weight automatically becomes 1.0 - (0.8 + 0.15) == 0.05 */);
	```

### Generating with dependencies

* `dependency<T,U>(gen1, gen2generator)`: generates a `std::pair<T,U>` with a generator `gen1` for type `T` and `gen2generator`. `gen2generator` receives a type `T` and returns a generator for type `U`.

	```cpp
	auto sizeAndVectorGen = dependency<int, std::vector<bool>>(Arbitrary<int>(),[](const int& num) {
	    auto vectorGen = Arbitrary<std::vector<int>>();
	    vectorGen.maxLen = num;
	    // generates a vector with maximum size of num
	    return vectorGen;
	});
	```

