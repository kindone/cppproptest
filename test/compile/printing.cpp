#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include <iostream>

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
    show(std::cout, 0);
    std::cout << std::endl;
}

TEST(Compile, printing_container)
{
    std::vector<int> vec{0, 1, 2};
    show(std::cout, vec);
    std::cout << std::endl;
}

TEST(Compile, printing_shared_ptr)
{
    auto ptr = std::make_shared<int>(5);
    show(std::cout, ptr);
    std::cout << std::endl;
}

TEST(Compile, printing_custom)
{
    Cat cat;
    show(std::cout, cat);
    std::cout << std::endl;
}

namespace proptest {

std::ostream& show(std::ostream& os, const Cat&)
{
    os << "Cat";
    return os;
}

}  // namespace proptest

TEST(Compile, printing_custom_defined)
{
    Cat cat;
    show(std::cout, cat);
    std::cout << std::endl;
}

TEST(Compile, printing_container_custom)
{
    MyContainer<int> container;
    show(std::cout, container);
    std::cout << std::endl;
}

template <typename T>
std::ostream& show(std::ostream& os, const MyContainer<T>&)
{
    os << "MyContainer";
    return os;
}

TEST(Compile, printing_container_custom_defined)
{
    MyContainer<int> container;
    show(std::cout, container);
    std::cout << std::endl;
}

TEST(Compile, printing_container_custom_element)
{
    std::vector<std::shared_ptr<Cat>> vec;
    auto ptr = std::make_shared<Cat>();
    vec.push_back(ptr);
    show(std::cout, vec);
    std::cout << std::endl;
}

namespace proptest {
namespace util {
template <>
struct ShowDefault<Car>
{
    static std::ostream& show(std::ostream& os, const Car&)
    {
        os << "Car";
        return os;
    }
};


template <typename T>
struct ShowDefault<CarLike<T>>
{
    static std::ostream& show(std::ostream& os, const CarLike<T>& carLike)
    {
        os << "CarLike(" << Show<T>(carLike.t) << ")";
        return os;
    }
};

}  // namespace util
}  // namespace proptest

TEST(Compile, printing_container_custom_element_defined)
{
    std::vector<std::shared_ptr<Car>> vec;
    auto ptr = std::make_shared<Car>();
    vec.push_back(ptr);
    show(std::cout, vec);
    std::cout << std::endl;
}

TEST(Compile, printing_custom_templated_defined)
{
    std::vector<std::shared_ptr<CarLike<Car>>> vec;
    auto ptr = std::make_shared<CarLike<Car>>(Car());
    vec.push_back(ptr);
    show(std::cout, vec);
    std::cout << std::endl;

}
