# Using and Defining Generators 

## `GenFunction<T>`

You use _generators_ to generate randomized arguments for properties. 

A generator is a callable (function, functor, or lambda) with following signature:

```cpp
// (Random&) -> Shrinkable<T>
```

This can be represented with (or coerced to) a standard function type, `std::function<Shrinkable<T>(Random&)>`. In `cppproptest`, this function type is aliased as `GenFunction<T>`. We will use this term throughout the documentation.

```cpp
template <typename T>
using GenFunction = std::function<Shrinkable<T>(Random&);
```

By the way, you can refer to [`Shrinkable`](doc/Shrinking.md) for its further detail, but you can basically treat it as a wrapper for a value of type `T` here. So a generator generates a value of type `T` from a random generator. A generator can be defined as functor or lambda, as you would prefer.  

```cpp
auto myIntGen = [](Random& rand) {
    int smallInt = rand.getRandomInt8();
    return make_shrinkable<int>(smallInt);
};
```

## `Generator<T>`

Template class `Generator<T>` is an abstract functor class that coerces to `GenFunction<T>`. A `Generator` gives access to some useful methods and you can wrap your generator callable with this to decorate with those methods. All generators and combinators of `cppproptest` produce decorated generators, so that you can use the utility methods with ease.

```cpp
auto myIntGen = Generator<int>([](Random& rand) {
    int smallInt = rand.getRandomInt8();
    return make_shrinkable<int>(smallInt);
});

// .filter and other utility methods can be used once the generator is wrapped with Generator<T>
auto evenGen = myIntGen.filter([](int& value) {
    return value % 2 == 0;
}); // generates even numbers only
```


## Arbitraries

An `Arbitrary` refers to globally defined default _generator_ for a type. You can additionaly define an `Arbitrary<T>` for your type `T`, if it isn't already defined yet. By defining an `Arbitrary`, you can omit the custom generator argument that would have been needed to be passed everytime you define a property for that type. Following shows an example for defining an `Arbitrary`. Note that it should be defined under `proptest` namespace in order to be accessible in the framework.

```cpp
namespace proptest { // you should define your Arbitrary<T> inside the namespace

struct Arbitrary<Car> : ArbitraryBase<Car> {
  Shrinkable<Car> operator()(Random& rand) {
    bool isAutomatic = rand.getRandomBool();
    return make_shrinkable<Car>(isAutomatic); // make_shrinkable creates a Car object by calling Car's constructor with 1 boolean parameter
  }
};

}
```

There are useful helpers for creating new generators from existing ones. 

`filter` is such a helper. It selectively generates values that satisfies a criteria function. Following is an even number generator from the integer `Arbitrary`.

```cpp
// generates any integers
auto anyIntGen = Arbitrary<int>();
// generates even integers
auto evenGen = anyIntGen.filter([](int& num) {
    return num % 2 == 0;
});
```

You can find the full list of such helpers in section [Utility methods in standard generators](#Utility-methods-in-standard-generators).

&nbsp;

## Arbitraries provided by `cppproptest`

Built-in generators are called Arbitraries. `cppproptest` provides a set of Arbitraries for immediate generation of types that are often used.

* `char` and `bool`
* Character type: `char`
* Integral types: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`
* Floating point types: `float`, `double`
* String types: 
        * `std::string` (defaults to generate ASCII character strings in \[0x01, 0x7F\] range)
	* `UTF8String` (a class which extends `std::string` and can be used to generate valid [UTF-8](https://en.wikipedia.org/wiki/UTF-8) strings by using `Arbitrary<UTF8String>`)
	* `CESU8String` (similar to UTF-8, but can be used to generate valid [CESU-8](https://en.wikipedia.org/wiki/CESU-8) strings)
	* `UTF16BEString` and `UTF16LEString` for [UTF-16](https://en.wikipedia.org/wiki/UTF-16) big and little endian strings. CESU-8 and Unicode types produce full unicode code point range of \[0x1, 0x10FFFF\], excluding forbidden surrogate code points (\[0xD800, 0xDFFF\])
* Shared pointers: `std::shared_ptr<T>` where an `Arbitrary<T>` or a custom generator for `T` is available. It's useful for generating polymorphic types.
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
		auto vecInt0to100 = Arbitrary<std::vector<int>>(interval<int>(0,100));
		// otherwise, Arbitrary<int> is used
		auto vecInt = Arbitrary<std::vector<int>>();
	        auto uppercaseGen = Arbitrary<std::string>(interval('A', 'Z'));
		```

	* `Arbitrary<std::Map>` provides setter methods for assigning key and value generators

		```cpp
		auto mapGen = Arbitrary<std::map<int,int>>();
		mapGen.setKeyGen(interval<int>(0,100)); // key ranges from 0 to 100
		mapGen.setElemGen(interval<int>(-100, 100)); // value ranges from -100 to 100
		```

   	* Containers provide methods for configuring desired sizes
		* `setMinSize(size)`, `setMaxSize(size)` for restricting to specific range of sizes
		* `setSize(size)` for restricting to specific size

		```cpp
		auto vecInt = Arbitrary<std::vector<int>>();
		vecInt.setSize(10); // 1) generated vector will always have size 10
		vecInt.setMinSize(1); // 2) generated vector will have size >= 1
		vecInt.setMaxSize(10); // generated vector will have size <= 10
                vecInt.setSize(1, 10); // 3) generated vector will have size >= 1 and size <= 10
		```

## Generator Combinators

Generator combinators are provided for building a new generator based on existing ones. They can be chained as they receive existing generator(s) as argument and returns new generator.

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
	auto pairGen = pair(Arbitrary<int>(), Arbitrary<std::string>());
	```

* `tuple<T1, ..., TN>(gen1, ..., genN)`: generates a `std::tuple<T1,...,TN>` based on result of generators `gen1` through `genN`

	```cpp
	auto tupleGen = tuple(Arbitrary<int>(), Arbitrary<std::string>(), Arbitrary<double>());
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

* `construct<T, ARG1, ..., ARGN>([gen1, ..., genM])`: generates an object of type `T` by calling its constructor that matches the signature `(ARG1, ..., ARGN)`. Custom generators `gen1`,..., `genM` can be supplied for generating arguments. If `M < N`, then rest of the arguments are generated with `Arbitrary`s.

	```cpp
	struct Coordinate {
	    Coordinate(int x, int y) {
		// ...
	    }
	};
	// ...
	auto coordinateGen1 = construct<Coordinate, int, int>(interval(-10, 10), interval(-20, 20));
	auto coordinateGen2 = construct<Coordinate, int, int>(interval(-10, 10)); // y is generated with Arbitrary<int>
	```

### Applying constraints

* `filter<T>(gen, condition_predicate)`:  generates a type `T` that satisfies condition predicate (`condition_predicate` returns `true`)

	```cpp
	// generates even numbers
	auto evenGen = filter<int>(Arbitrary<int>(),[](int& num) {
	    return num % 2 == 0;
	});
	```
* `suchThat<T>`: an alias of `filter`

### Transformation or mapping

* `transform<T,U>(gen, transformer)`: generates type `U` based on generator for type `T`, using `transformer` that transforms a value of type `T` to type `U`

	```cpp
	// generates string from integers (e.g. "0", "1", ... , "-16384")
	auto numStringGen = transform<int, std::string>(Arbitrary<int>(),[](int& num) {
	    return std::string(num);
	});
	```

### Generating with dependencies

* `derive<T, U>(genT, genUGen)`: derives a new generator for type `U`, based on result of `genT`, which is a generator for type `T`. Difference to `transform<T,U>`) is that you can have greater control on the resultant generator.
	
	```cpp
	// generates a string something like "KOPZZFASF", "ghnpqpojv", or "49681002378", ... that consists of only uppercase/lowercase alphabets/numeric characters.
	auto stringGen = derive<int, std::string>(integers(0, 2), [](int& num) {
	    if(num == 0)
	        return Arbitrary<std::string>(interval('A', 'Z'));
	    else if(num == 1)
	        return Arbitrary<std::string>(interval('a', 'z'));
	    else // num == 2
	        return Arbitrary<std::string>(interval('0', '9'));
	});	
	```

* `dependency<T,U>(genT, genUgen)`: generates a `std::pair<T,U>` with a generator `genT` for type `T` and `genUgen`. `genUgen` receives a type `T` and returns a generator for type `U`. This can effectively create a generator for a pair where second item depends on the first one.

	```cpp
	auto sizeAndVectorGen = dependency<int, std::vector<bool>>(Arbitrary<bool>(), [](int& num) {
	    auto vectorGen = Arbitrary<std::vector<int>>();
	    vectorGen.maxLen = num;
	    // generates a vector with maximum size of num
	    return vectorGen;
	});

	auto nullableIntegers = dependency<bool, int>(Arbitrary<bool>(), [](bool& isNull) {
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

### Utility methods in standard generators

Standard generators and combinators returns `Generator<T>`, which is of the form `(Random&) -> Shrinkable<T>`, but has additional combinator methods decorated for ease of use. `Arbitrary<T>` and `Construct<...>` share the same property. As the combinators are also decorated, they can be chained multiple times.

* `.map<U>(mapper)`: effectively calls `transform<T,U>(gen, transformer)` combinator on itself with type `T` and generator `gen`.

	```cpp
	// generator for strings of arbitrary number
	Arbitrary<int>().map<std::string>([](int &num) {
	    return std::to_string(num);
	});
	// this is equivalent to:
	transform<int, std::string>(Arbitrary<int>(), [](int &num) {
	    return std::to_string(num);
	});
	```

* `.filter(filterer)`: apply `filter` combinator on itself.

	```cpp
	// two equivalent ways to generate random even numbers
	auto evenGen = Arbitrary<int>().filter([](int& num) {
	    return num % 2 == 0;
	});
	
	auto evenGen = filter<int>(Arbitrary<int>(),[](int& num) {
	    return num % 2 == 0;
	});
	```
	
* `.flatMap<U>(genUGen)`: based on generated result of the generator object itself, induces a new generator for type `U`. It's equivalent combinator is `derive<T,U>`. Difference to `.map<U>` (or `transform<T,U>`) is that you can have greater control on the resultant generator.
	
	```cpp
	auto stringGen = Arbitrary<int>().flatMap<std::string>([](int& num) {
	    auto genString = Arbitrary<std::string>();
	    genString.setMaxSize(num);
	    return genString;
	});	
	```

* `.pair<U>(genUGen)` or `tuple<U>(genUGen)`: chains itself to create a generator of pair or tuple. Effectively calls `dependency` or `chain`, respectively.
	```cpp
	Arbitrary<bool>().tuple<int>([](bool& isEven) {
	    if(isEven)
	        return Arbitrary<int>().filter([](int& value) {
	            return value % 2 == 0;
		});
	    else
	        return Arbitrary<int>().filter([](int& value) {
	            return value % 2 == 1;
		});
	}).tuple<std::string>([](std::tuple<bool, int>& tuple) {
	    int size = std::get<1>(tuple);
	    auto stringGen = Arbitrary<std::string>();
	    stringGen.setSize(size);
	    return stringGen;
	});
	```


