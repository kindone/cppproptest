
# Generator Combinators

Generator combinators are provided for building a new generator based on existing ones. They can be chained as they receive existing generator(s) as argument and returns new generator.

### Constants

* `just<T>(T*)` or `just<T>(T)`: always generates specific value
* `lazy<T>(std::function<T()>)`: generates a value by calling a function
	```cpp
	auto zeroGen = just(0); // template argument is optional
	auto oneGen = lazy<int>([]() { return 1; });
	```

### Integers and intervals

Some utility generators for integers are provided by `cppproptest`
* `interval<INT_TYPE>(min, max)`: generates an integer type(e.g. `uint16_t`) in the closed interval `[min, max]`.
* `integers<INT_TYPE(from, count)`: generates an integer type starting from `from`
	```cpp
	interval<int64_t>(1, 28);
	interval(1, 48); // template type argument can be ommitted if the input type(`int`) is the same as the output type.
	interval(1L, 48L); // template type argument can be ommitted if the input type(`int`) is the same as the output type.
	interval(0, 10); // generates an integer in {0, ..., 10}
	interval('A', 'Z'); // generates a char of uppercase alphabet
	integers(0, 10); // generates an integer in {0, ..., 9}
	integers(1, 10); // generates an integer in {1, ..., 10}
	```
* `natural<INT_TYPE>(max)`: generates a positive integer up to `max`(inclusive)
* `nonNegative<INT_TYPE>(max)`: : generates zero or a positive integer up to `max`(inclusive)

### Selecting from values

* `elementOf<T>(val1, ..., valN)`: generates a type `T` from multiple values for type `T`, by choosing one of the values randomly
	```cpp
	// generates a prime number under 100
	auto primeGen = oneOf<int>(2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97);
	```

	* `elementOf` can receive optional probabilitistic weights (`0 < weight < 1`, sum of weights must not exceed 1.0) for generators. If weight is unspecified for a generator, it is calculated automatically so that remaining probability among unspecified generators is evenly distributed.
	`weightedVal(<value>, <weight>)` is used to annotate the desired weight. 

	```cpp
	// generates a numeric within ranges [0,10], [100, 1000], [10000, 100000]
	elementOf<int>(weightedVal(2, 0.8), weightedVal(5, 0.15), 10/* weight automatically becomes 1.0 - (0.8 + 0.15) == 0.05 */);
	```

### Pair and Tuples

* `pair<T1, T2>(gen1, gen2)` : generates a `std::pair<T1,T2>` based on result of generators `gen1` and `gen2`

	```cpp
	auto pairGen = pair(Arbi<int>(), Arbi<std::string>());
	```

* `tuple<T1, ..., TN>(gen1, ..., genN)`: generates a `std::tuple<T1,...,TN>` based on result of generators `gen1` through `genN`

	```cpp
	auto tupleGen = tuple(Arbi<int>(), Arbi<std::string>(), Arbi<double>());
	```

### Selecting from generators

* `oneOf<T>(gen1, ..., genN)`: generates a type `T` from multiple generators for type `T`, by choosing one of the generators randomly

	```cpp
	// generates a numeric within ranges [0,10], [100, 1000], [10000, 100000]
	auto evenGen = oneOf<int>(interval(0, 10), interval(100, 1000), interval(10000, 100000));
	```

	* `oneOf` can receive optional probabilitistic weights (`0 < weight < 1`, sum of weights must not exceed 1.0) for generators. If weight is unspecified for a generator, it is calculated automatically so that remaining probability among unspecified generators is evenly distributed.
	`weightedGen(<generator>, <weight>)` is used to annotate the desired weight. 

	```cpp
	// generates a numeric within ranges [0,10], [100, 1000], [10000, 100000]
	auto evenGen = oneOf<int>(weightedGen(interval(0, 10), 0.8), weightedGen(interval(100, 1000), 0.15), interval(10000, 100000)/* weight automatically becomes 1.0 - (0.8 + 0.15) == 0.05 */);
	```

### Constructing an object

* `construct<T, ARG1, ..., ARGN>([gen1, ..., genM])`: generates an object of type `T` by calling its constructor that matches the signature `(ARG1, ..., ARGN)`. Custom generators `gen1`,..., `genM` can be supplied for generating arguments. If `M < N`, then rest of the arguments are generated with `Arbi`s.

	```cpp
	struct Coordinate {
	    Coordinate(int x, int y) {
		// ...
	    }
	};
	// ...
	auto coordinateGen1 = construct<Coordinate, int, int>(interval(-10, 10), interval(-20, 20));
	auto coordinateGen2 = construct<Coordinate, int, int>(interval(-10, 10)); // y is generated with Arbi<int>
	```

### Applying constraints

* `filter<T>(gen, condition_predicate)`:  generates a type `T` that satisfies condition predicate (`condition_predicate` returns `true`)

	```cpp
	// generates even numbers
	auto evenGen = filter<int>(Arbi<int>(),[](int& num) {
	    return num % 2 == 0;
	});
	```
* `suchThat<T>`: an alias of `filter`

### Transforming or mapping

* `transform<T,U>(gen, transformer)`: generates type `U` based on generator for type `T`, using `transformer` that transforms a value of type `T` to type `U`

	```cpp
	// generates string from integers (e.g. "0", "1", ... , "-16384")
	auto numStringGen = transform<int, std::string>(Arbi<int>(),[](int& num) {
	    return std::string(num);
	});
	```

### Values with dependencies

* `derive<T, U>(genT, genUGen)`: derives a new generator for type `U`, based on result of `genT`, which is a generator for type `T`. Difference to `transform<T,U>`) is that you can have greater control on the resultant generator.
	
	```cpp
	// generates a string something like "KOPZZFASF", "ghnpqpojv", or "49681002378", ... that consists of only uppercase/lowercase alphabets/numeric characters.
	auto stringGen = derive<int, std::string>(integers(0, 2), [](int& num) {
	    if(num == 0)
	        return Arbi<std::string>(interval('A', 'Z'));
	    else if(num == 1)
	        return Arbi<std::string>(interval('a', 'z'));
	    else // num == 2
	        return Arbi<std::string>(interval('0', '9'));
	});	
	```

* `dependency<T,U>(genT, genUgen)`: generates a `std::pair<T,U>` with a generator `genT` for type `T` and `genUgen`. `genUgen` receives a type `T` and returns a generator for type `U`. This can effectively create a generator for a pair where second item depends on the first one.

	```cpp
	auto sizeAndVectorGen = dependency<int, std::vector<bool>>(Arbi<bool>(), [](int& num) {
	    auto vectorGen = Arbi<std::vector<int>>();
	    vectorGen.maxLen = num;
	    // generates a vector with maximum size of num
	    return vectorGen;
	});

	auto nullableIntegers = dependency<bool, int>(Arbi<bool>(), [](bool& isNull) {
	    if(isNull)
		return just<int>([]() { return 0; });
	    else
		return fromTo<int>(10, 20);
	});
	```

* `chain<Ts..., U>(genT, genUgen)`: similar to `dependency`, but takes a tuple generator for `std::tuple<Ts...>` and generates a `std::tuple<Ts..., U>` instead of a `std::pair`. `chain` can be repeatedly applied to itself, and results in a tuple one element larger than the previous one. You can chain multiple dependencies with this form.

	```cpp
	auto yearMonthGen = tuple(fromTo(0, 9999), fromTo(1,12));
	// number of days of month depends on month (28~31 days) and year (whether it's a leap year)
	auto yearMonthDayGen = chain<std::tuple<int, int>, int>(yearMonthGen, [](std::tuple<int,int>& yearMonth) {
	    int year = std::get<0>(yearMonth);
	    int month = std::get<1>(yearMonth);
	    if(monthHas31Days(month)) {
	        return fromTo(1, 31);
	    }
	    else if(monthHas30Days(month)) {
	        return fromTo(1, 30);
	    }
	    else { // february has 28 or 29 days
	        if(isLeapYear(year))
		    return fromTo(1, 29);
		else
		    return fromTo(1, 28);
	    }
	}); // yearMonthDayGen generates std::tuple<int, int, int> of (year, month, day)
	```

## Utility methods in standard generators

Standard generators and combinators (including `Arbi<T>` and `Construct<...>`) returns a `Generator<T>`, which is of the form `(Random&) -> Shrinkable<T>` (aliased as `GenFunction<T>`), but has additional combinator methods decorated for ease of use. They can be chained multiple times.

* `.map<U>(mapper)`: effectively calls `transform<T,U>(gen, transformer)` combinator on itself with type `T` and generator `gen`.

	```cpp
	// generator for strings of arbitrary number
	Arbi<int>().map<std::string>([](int &num) {
	    return std::to_string(num);
	});
	// this is equivalent to:
	transform<int, std::string>(Arbi<int>(), [](int &num) {
	    return std::to_string(num);
	});
	```

* `.filter(filterer)`: apply `filter` combinator on itself.

	```cpp
	// two equivalent ways to generate random even numbers
	auto evenGen = Arbi<int>().filter([](int& num) {
	    return num % 2 == 0;
	});
	
	auto evenGen = filter<int>(Arbi<int>(),[](int& num) {
	    return num % 2 == 0;
	});
	```
	
* `.flatMap<U>(genUGen)`: based on generated result of the generator object itself, induces a new generator for type `U`. It's equivalent combinator is `derive<T,U>`. Difference to `.map<U>` (or `transform<T,U>`) is that you can have greater control on the resultant generator.
	
	```cpp
	auto stringGen = Arbi<int>().flatMap<std::string>([](int& num) {
	    auto genString = Arbi<std::string>();
	    genString.setMaxSize(num);
	    return genString;
	});	
	```

* `.pair<U>(genUGen)` or `tuple<U>(genUGen)`: chains itself to create a generator of pair or tuple. Effectively calls `dependency` or `chain`, respectively.
	```cpp
	Arbi<bool>().tuple<int>([](bool& isEven) {
	    if(isEven)
	        return Arbi<int>().filter([](int& value) {
	            return value % 2 == 0;
		});
	    else
	        return Arbi<int>().filter([](int& value) {
	            return value % 2 == 1;
		});
	}).tuple<std::string>([](std::tuple<bool, int>& tuple) {
	    int size = std::get<1>(tuple);
	    auto stringGen = Arbi<std::string>();
	    stringGen.setSize(size);
	    return stringGen;
	});
	```


