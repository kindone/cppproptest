#include "proptest/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "proptest/Random.hpp"

using namespace proptest;

struct Cat
{
};

struct Car
{
};

template <typename T>
struct CarLike
{
    CarLike(const T& _t) : t(_t) {}

    T t;
};

template <typename T>
struct MyContainer
{
};

TEST(Compile, printing_primitive)
{
    show(cout, 0);
    cout << endl;
}

TEST(Compile, printing_container)
{
    vector<int> vec{0, 1, 2};
    show(cout, vec);
    cout << endl;
}

TEST(Compile, printing_shared_ptr)
{
    auto ptr = util::make_shared<int>(5);
    show(cout, ptr);
    cout << endl;
}

TEST(Compile, printing_custom)
{
    Cat cat;
    show(cout, cat);
    cout << endl;
}

namespace proptest {

ostream& show(ostream& os, const Cat&)
{
    os << "Cat";
    return os;
}

}  // namespace proptest

TEST(Compile, printing_custom_defined)
{
    Cat cat;
    show(cout, cat);
    cout << endl;
}

TEST(Compile, printing_container_custom)
{
    MyContainer<int> container;
    show(cout, container);
    cout << endl;
}

template <typename T>
ostream& show(ostream& os, const MyContainer<T>&)
{
    os << "MyContainer";
    return os;
}

TEST(Compile, printing_container_custom_defined)
{
    MyContainer<int> container;
    show(cout, container);
    cout << endl;
}

TEST(Compile, printing_container_custom_element)
{
    vector<shared_ptr<Cat>> vec;
    auto ptr = util::make_shared<Cat>();
    vec.push_back(ptr);
    show(cout, vec);
    cout << endl;
}

namespace proptest {
namespace util {
template <>
struct ShowDefault<Car>
{
    static ostream& show(ostream& os, const Car&)
    {
        os << "Car";
        return os;
    }
};


template <typename T>
struct ShowDefault<CarLike<T>>
{
    static ostream& show(ostream& os, const CarLike<T>& carLike)
    {
        os << "CarLike(" << Show<T>(carLike.t) << ")";
        return os;
    }
};

}  // namespace util
}  // namespace proptest

TEST(Compile, printing_container_custom_element_defined)
{
    vector<shared_ptr<Car>> vec;
    auto ptr = util::make_shared<Car>();
    vec.push_back(ptr);
    show(cout, vec);
    cout << endl;
}

TEST(Compile, printing_custom_templated_defined)
{
    vector<shared_ptr<CarLike<Car>>> vec;
    auto ptr = util::make_shared<CarLike<Car>>(Car());
    vec.push_back(ptr);
    show(cout, vec);
    cout << endl;

}
