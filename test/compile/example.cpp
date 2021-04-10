#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"

using namespace proptest;

TEST(Example, DefaultGen)
{
    forAll([](int i, double d, std::string str, std::vector<uint64_t> vec) {
        // i, d .....
    });
}

TEST(Example, CustomGen)
{
    auto intGen = elementOf<int>(2, 4, 6);

    forAll([](int num) {
        std::cout << num << std::endl;
    }, intGen);
}

TEST(Example, property)
{
    auto prop = property([](std::string name, int num) {
        std::cout << "name: " << name << ", num: " << num << std::endl;
    });

    prop.forAll();
    prop.matrix({"Alice", "Bob"}, {0, 1});
    prop.example("Charlie", 2);
}


TEST(Example, TemplatedGen)
{
    auto intGen = interval<int>(2, 100000);
    auto stringIntGen = intGen.map([](int& num) {
        return std::to_string(num);/// "2", "4", "6" 
    });

    forAll([](std::string str, std::vector<std::string> numStrings) {
        //std::cout << str << std::endl; 
        std::cout << "[ ";
        if(numStrings.size() > 0)
            std::cout << numStrings[0]; 
        for(size_t i = 1; i < numStrings.size(); i++)
            std::cout << ", " << numStrings[i]; 
        std::cout << " ]" << std::endl;

    }, stringIntGen, Arbi<std::vector<std::string>>(stringIntGen) );
}

TEST(Example, MapGen)
{
    auto intGen = elementOf<int>(2, 4, 6);
    auto stringIntGen = intGen.map([](int& num) {
        return std::to_string(num);
    });

    Arbi<std::map<std::string, std::string>> mapGen;

    forAll([](std::string str, std::map<std::string, std::string> nameAgeMap) {
        //std::cout << str << std::endl; 
    }, stringIntGen, mapGen.setElemGen(stringIntGen).setMaxSize(3));
}
