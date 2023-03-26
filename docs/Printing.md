# Printing Facilities

It's sometimes recommended to have a printer defined for a type. It would ensure values of a type to be correctly printed. `cppproptest` fully defines printers for built-in types.

```cpp
// a std::tuple<std::tuple<int, std::tuple<int>>> type is printed:
shrinking found simpler failing arg 0: { { -10, { -1002144 } } }
```

For a new type you'd like to use with `cppproptest`, you can define a printer for that type. As you can see in above example, complex recursive structures such as tuple of tuples can be printed if you have correctly defined it. If there is no printer is defined for a type yet, '???' would be printed instead, as there is no correct way known to `cppproptest` how to printing that type .

```cpp
// Car type that does not yet have a printing method, so it's printed as '???':
shrinking found simpler failing arg 0: ???
```

### Defining a printer for a type

Defining a printer for type `T` can be achieved by defining the struct specialization `proptest::util::ShowDefault<T>`:

```cpp
namespace proptest {
namespace util {

// printer for Car class
template <>
struct ShowDefault<Car>
{
    static std::ostream& show(std::ostream& os, const Car& car)
    {
        os << "Car(" << car.name << ", " << car.year << ")";
        return os;
    }
};

} // namespace util
} // namespace proptest
```

This will be called whenever `T` is being printed by `cppproptest`:

```cpp
// Car type now has a printer defined
shrinking found simpler failing arg 0: Car(Ferari, 2020)
```

### Defining a printer for a templated type

You can also define a printer for a templated type as following:

```cpp
namespace proptest {
namespace util {

// printer for CarLike<T>
template <typename T>
struct ShowDefault<CarLike<T>>
{
    static std::ostream& show(std::ostream& os, const CarLike<T>&)
    {
        os << "CarLike()";
        return os;
    }
};

} // namespace util
} // namespace proptest
```

This will print a `CarLike` as:

```cpp
shrinking found simpler failing arg 0: CarLike()
```


### Utilizing other printers

If your printer fneeds to print another type `T`, you can use an already defined (either built-in or custom one) printer for `T` by calling `proptest::util::Show<T>(T&)`, as following:

```cpp
namespace proptest {
namespace util {

template <typename T>
struct ShowDefault<CarLike<T>>
{
    static std::ostream& show(std::ostream& os, const CarLike<T>& carLike)
    {
        // utilize printer for type T ('CarLike(???)' will be printed if there is no printer defined for T)
        os << "CarLike(" << Show<T>(carLike.t) << ")";
        return os;
    }
};

} // namespace util
} // namespace proptest
```

Now it will correctly prints a `CarLike<Car>`:

```cpp
shrinking found simpler failing arg 0: CarLike(Car(Ferari, 2020))
```
