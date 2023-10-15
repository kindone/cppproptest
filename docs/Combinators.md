
# Generator Combinators

Generator combinators are provided for building a new generator based on existing ones. Many of them come from ideas and best practices of functional programming. They can be chained as they receive existing generator(s) as argument and returns new generator.

While you can go through this document from top to the bottom, you might be want to find a suitable combinator for your use case using this table:

| Purpose                                              | Related Generator/Combinator                                                                                           | Examples                                                                      |
| ---------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------- |
| Generate just a constant                             | [`just<T>`](#constants)                                                                                                | `0` or `"1337"`                                                               |
| Generate a value within constants                    | [`elementOf<T>`](#selecting-from-values)                                                                               | a prime number under 100                                                      |
| Generate a list of unique values                     | [`Arbi<set<T>>`](Generators.md#built-in-arbitraries)                                                                   | `{3,5,1}` but not `{3,5,5}`                                                   |
| Generate a value within numeric range of values      | [`interval<T>`, `integers<T>`](#integers-and-intervals)                                                                | a number within `1`~`9999`                                                    |
| Generate a pair or a tuple of different types        | [`pairOf<T1,T2>`, `tupleOf<Ts...>`](#pair-and-tuples)                                                                  | a `pair<int, string>`                                                         |
| Union multiple generators                            | [`unionOf<T>` (`oneOf<T>`)](#selecting-from-generators)                                                                | `20~39` or `60~79` combined                                                   |
| Transform into another type or a value               | [`transform<T,U>`](#transforming-or-mapping)                                                                           | `"0"` or `"1.4"` (a number as string).                                        |
| Generate a struct or a class object                  | [`construct<T,ARGS...>`](#constructing-an-object)                                                                      | a `Rectangle` object with width and height                                    |
| Apply constraints in generated values                | [`filter` (`suchThat`)](#applying-constraints)                                                                         | an even natural number (`n % 2 == 0`)                                         |
| Generate values with dependencies or relationships   | [`dependency`, `chain`](#values-with-dependencies), [`pairWith`, `tupleWith`](#utility-methods-in-standard-generators) | a rectangle where `width == height * 2`                                       |
| Generate a value based on previously generated value | [`aggregate`, `accumulate`](#aggregation-or-accumulation-of-values)                                                    | a sequence of numbers where each one is between 0.5x and 1.5x of its previous |

&nbsp;

## Basic Generator Combinators

### Constants

* `just<T>(T*)`, `just<T>(T)`, `just<T>(shared_ptr<T>)`: always generates a specific value. A shared pointer can be used for non-copyable types.
* `lazy<T>(std::function<T()>)`: generates a value by calling a function
    ```cpp
    auto zeroGen = just(0); // template argument is optional if type is deducible
    auto oneGen = lazy<int>([]() { return 1; });
    ```

### Selecting from constants

You may want to random choose from specific list of values.

* `elementOf<T>(val1, ..., valN)`: generates a type `T` from multiple values for type `T`, by choosing one of the values randomly
    ```cpp
    // generates a prime number under 50
    auto primeGen = elementOf<int>(2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47);
    ```

    * `elementOf` can receive optional probabilitistic weights (`0 < weight < 1`, sum of weights must not exceed 1.0) for generators. If weight is unspecified for a generator, it is calculated automatically so that remaining probability among unspecified generators is evenly distributed.
    `weightedVal(<value>, <weight>)` is used to annotate the desired weight.

    ```cpp
    // generates a numeric within ranges [0,10], [100, 1000], [10000, 100000]
    //   weight for 10 automatically becomes 1.0 - 0.8 - 0.15 == 0.05
    elementOf<int>(weightedVal(2, 0.8), weightedVal(5, 0.15), 10);
    ```

### Integers and intervals

Some utility generators for integers are provided

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

### Pair and Tuples

Generators for different types can be bound to a pair or a tuple.

* `pairOf<T1, T2>(gen1, gen2)` : generates a `std::pair<T1,T2>` based on result of generators `gen1` and `gen2`

    ```cpp
    auto pairGen = pairOf(Arbi<int>(), Arbi<std::string>());
    ```

* `tupleOf<T1, ..., TN>(gen1, ..., genN)`: generates a `std::tuple<T1,...,TN>` based on result of generators `gen1` through `genN`

    ```cpp
    auto tupleGen = tupleOf(Arbi<int>(), Arbi<std::string>(), Arbi<double>());
    ```

&nbsp;

## Advanced Generator Combinators

### Selecting from generators

You can combine generators to a single generator that can generate each of them with some probability. This can be considered as taking a *union* of generators.

* `oneOf<T>(gen1, ..., genN)`: generates a type `T` from multiple generators for type `T`, by choosing one of the generators randomly

    ```cpp
    // generates a numeric within ranges [0,10], [100, 1000], [10000, 100000]
    auto evenGen = oneOf<int>(interval(0, 10), interval(100, 1000), interval(10000, 100000));
    ```

    * `oneOf` can receive optional probabilistic weights (`0 < weight < 1`, sum of weights must not exceed 1.0) for generators. If weight is unspecified for a generator, it is calculated automatically so that remaining probability among unspecified generators is evenly distributed.
    `weightedGen(<generator>, <weight>)` is used to annotate the desired weight.

    ```cpp
    // generates a numeric within ranges [0,10], [100, 1000], [10000, 100000]
    auto evenGen = oneOf<int>(weightedGen(interval(0, 10), 0.8), weightedGen(interval(100, 1000), 0.15), interval(10000, 100000)/* weight automatically becomes 1.0 - (0.8 + 0.15) == 0.05 */);
    ```

* `unionOf<T>` is an alias of `oneOf<T>`

### Constructing an object

You can generate an object of a class or a struct type `T`, by calling a matching constructor of `T`.

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

You can add a filtering condition to a generator to restrict the generated values to have certain constraint.

* `filter<T>(gen, condition_predicate)`:  generates a type `T` that satisfies condition predicate (`condition_predicate` returns `true`)

    ```cpp
    // generates even numbers
    auto evenGen = filter<int>(Arbi<int>(),[](int& num) {
        return num % 2 == 0;
    });
    ```

* `suchThat<T>`: an alias of `filter`

### Transforming or mapping

You can transform an existing generator to create new generator by providing a transformer function. This is equivalent to *mapping* in functional programming context.

* `transform<T,U>(gen, transformer)`: generates type `U` based on generator for type `T`, using `transformer` that transforms a value of type `T` to type `U`

    ```cpp
    // generates string from integers (e.g. "0", "1", ... , "-16384")
    auto numStringGen = transform<int, std::string>(Arbi<int>(),[](int& num) {
        return std::string(num);
    });
    ```

### Deriving or flat-mapping

Another combinator that resembles `transform` is `derive`. This is equivalent to *flat-mapping* or *binding* in functional programming. Difference to `transform<T,U>` is that you can have greater control on the resultant generator.

* `derive<T, U>(genT, genUGen)`: derives a new generator for type `U`, based on result of `genT`, which is a generator for type `T`.

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

Following table compares `transform` and `derive`:

| Combinator       | transformer signature       | Result type    |
| ---------------- | --------------------------- | -------------- |
| `transform<T,U>` | `function<U(T)>`            | `Generator<U>` |
| `derive<T,U>`    | `function<Generator<U>(T)>` | `Generator<U>` |



### Values with dependencies

You may want to include dependency in the generated values. There are two variants that do this. One generates a pair and the other one generates a tuple.

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
        return just<int>(0);
        else
        return fromTo<int>(10, 20);
    });
    ```

* `chain<Ts..., U>(genT, genUgen)`: similar to `dependency`, but takes a tuple generator for `std::tuple<Ts...>` and generates a `std::tuple<Ts..., U>` instead of a `std::pair`. `chain` can be repeatedly applied to itself, and results in a tuple one element larger than the previous one. You can chain multiple dependencies with this form.

    ```cpp
    auto yearMonthGen = tupleOf(fromTo(0, 9999), fromTo(1,12));
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

Actually you can achieve the similar goal using `filter` combinator:

```cpp
    // generate any year,month,day combination
    auto yearMonthDayGen = tupleOf(fromTo(0, 9999), fromTo(1,12), fromTo(1,31));
    // apply filter
    auto validYearMonthDayGen = yearMonthDayGen.filter([](std::tuple<int,int,int>& ymd) {
        int year = std::get<0>(ymd);
        int month = std::get<1>(ymd);
        int day = std::get<2>(ymd);
        if(monthHas31Days(month) && day <= 31)
            return true;
        else if(monthHas30Days(month) && day <= 30)
            return true;
        else { // february has 28 or 29 days
            if(isLeapYear(year) && day <= 29)
                return true;
            else
                return day <= 28;
        }
    });
```

However, using `filter` for generating values with complex dependency may result in many generated values that do not meet the constraint to be discarded and retried. Therefore it's usually not recommended for that purpose if the ratio of discarded values is high.


### Aggregation or Accumulation of Values

You may want to generate values that are related to previously generated values. This can be achieved with `aggregate` or `accumulate`.
Both of the combinators take base generator in the form of `Generator<T>` as the first argument and a factory that takes a value of type `T` and returns `Generator<T>`, as the second argument.

While `aggregate` generates a single value, accumulate generates a list of values at each generation.

| Combinator                                                      | Result type          | Remark |
| --------------------------------------------------------------- | -------------------- | ------ |
| `aggregate<GenT, GenT2GenT>(genT, gen2GenT, minSize, maxSize)`  | `Generator<T>`       |        |
| `accumulate<GenT, GenT2GenT>(genT, gen2GenT, minSize, maxSize)` | `Generator<list<T>>` |        |

```cpp
    // generate initial value
    auto baseGen = interval(0, 1000);
    // generate a value based on previous value
    auto gen = aggregate(
        gen1,
        [](int& num) {
            return interval(num/2, num*2);
        },
        2 /* min size */, 10 /* max size */);
```

```cpp
    // generate initial value
    auto baseGen = interval(0, 1000);
    // generate list of values
    auto gen = accumulate(
        gen1,
        [](int& num) {
            return interval(num/2, num*2);
        },
        2 /* min size */, 10 /* max size */);
```

## Utility Methods in Standard Generators

Standard generators and combinators (including `Arbi<T>` and `Construct<...>`) returns a `Generator<T>`, which is of the form `(Random&) -> Shrinkable<T>` (aliased as `GenFunction<T>`), but has additional combinator methods decorated for ease of use. They in fact have equivalent standalone counterparts. Following table shows this relationship:

| Decorated method                             | Result type                      | Equivalent Standalone combinator |
| -------------------------------------------- | -------------------------------- | -------------------------------- |
| `Generator<T>::filter`                       | `Generator<T>`                   | `filter<T>`                      |
| `Generator<T>::map<U>`                       | `Generator<U>`                   | `transform<T,U>`                 |
| `Generator<T>::flatMap<U>`                   | `Generator<U>`                   | `derive<T,U>`                    |
| `Generator<T>::pairWith<U>`                  | `Generator<std::pair<T,U>>`      | `dependency<T,U>`                |
| `Generator<T>::tupleWith<U>`                 | `Generator<std::tuple<T,U>>`     | `chain<T,U>`                     |
| `Generator<std::tuple<Ts...>>::tupleWith<U>` | `Generator<std::tuple<Ts...,U>>` | `chain<std::tuple<Ts...>,U>`     |

These functions and methods can be continuously chained.

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

* `.pairWith<U>(genUGen)` or `tupleWith<U>(genUGen)`: chains itself to create a generator of pair or tuple. Equivalent to `dependency` or `chain`, respectively.
    ```cpp
    Arbi<bool>().tupleWith<int>([](bool& isEven) {
        if(isEven)
            return Arbi<int>().filter([](int& value) {
                return value % 2 == 0;
        });
        else
            return Arbi<int>().filter([](int& value) {
                return value % 2 == 1;
        });
    }).tupleWith<std::string>([](std::tuple<bool, int>& tuple) {
        int size = std::get<1>(tuple);
        auto stringGen = Arbi<std::string>();
        stringGen.setSize(size);
        return stringGen;
    });
    ```

    Notice `tupleWith` can automatically chain a tuple generator of `n` parameters into a tuple generator of `n+1` parameters (`bool` generator -> `tuple<bool, int>` generator -> `tuple<bool, int, string>` generator in above example)
