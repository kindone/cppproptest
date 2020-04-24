# Using and Defining Generators 

## Arbitraries

The framework provides `Arbitrary<T>` for following primitive types
* Integral types: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
* Floating point types: `float`, `double`
* String types: `std::string`, `UTF8String`
* Standard containers: `std::vector`, `std::pair`, `std::tuple`

## Generator Combinators

Generator combinators are provided for building a new generator based on existing ones. They can be chained as they receive existing generator(s) as argument and returns new generator.

* `just<T>(T*)` or `just<T>(function<T()>)`: always generates specific value

* `pair<T1, T2>(gen1, gen2)` : generates a `std::pair<T1,T2>` based on result of generators `gen1` and `gen2`

	```cpp
	auto pairGen = pair<int, std::string>(Arbitrary<int>(), Arbitrary<std::string>());
	```

* `tuple<T1, ..., TN>(gen1, ..., genN)`: generates a `std::tuple<T1,...,TN>` based on result of generators `gen1` through `genN`

	```cpp
	auto tupleGen = tuple<int, std::string, double>(Arbitrary<int>(), Arbitrary<std::string>(), Arbitrary<double>());
	```

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

* `suchThat<T>` or `filter<T>(gen, condition_predicate)`:  generates a type `T` that satisfies condition predicate (`condition_predicate` returns `true`)

	```cpp
	// generates even numbers
	auto evenGen = filter<int>(Arbitrary<int>(),[](const int& num) {
	    return num % 2 == 0;
	});
	```

* `transform<T,U>(gen, transformer)`: generates type `U` based on generator for type `T`, using `transformer` that transforms a value of type `T` to type `U`

	```cpp
	// generates string from integers (e.g. "0", "1", ... , "-16384")
	auto numStringGen = transform<int, std::string>(Arbitrary<int>(),[](const int& num) {
	    return std::string(num);
	});
	```

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

* `dependency<T,U>(gen1, gen2generator)`: generates a `std::pair<T,U>` with a generator `gen1` for type `T` and `gen2generator`. `gen2generator` receives a type `T` and returns a generator for type `U`.

	```cpp
	auto sizeAndVectorGen = dependency<int, std::vector<bool>>(Arbitrary<int>(),[](const int& num) {
	    auto vectorGen = Arbitrary<std::vector<int>>();
	    vectorGen.maxLen = num;
	    // generates a vector with maximum size of num
	    return vectorGen;
	});
	```

