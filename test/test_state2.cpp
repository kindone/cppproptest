#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include <chrono>
#include <iostream>
#include <memory>

using namespace proptest;

template <typename SystemType>
using ActionFor = std::function<bool(SystemType&)>;

template <typename SystemType>
decltype(auto) actionFor(ActionFor<SystemType> func)
{
    auto funcPtr = std::make_shared<decltype(func)>(func);
    return just<ActionFor<SystemType>>([funcPtr]() { return *funcPtr; });
}

template <typename SystemType, typename... GENS>
std::function<Shrinkable<std::vector<std::function<bool(SystemType&)>>>(Random&)> actions2(GENS&&... gens)
{
    auto actionGen = oneOf<std::function<bool(SystemType&)>>(std::forward<GENS>(gens)...);
    auto actionVecGen = Arbitrary<std::vector<std::function<bool(SystemType&)>>>(actionGen);
    return actionVecGen;
}

template <typename SystemType, typename... GENS>
decltype(auto) statefulProperty2(GENS&&... gens)
{
    auto actionsGen = actions2<SystemType>(std::forward<GENS>(gens)...);

    return property(
        [](SystemType obj, std::vector<std::function<bool(SystemType&)>> actions) {
            for (auto action : actions) {
                PROP_ASSERT(action(obj));
            }
            return true;
        },
        Arbitrary<SystemType>(), actionsGen);
}

TEST(StateTest, States2)
{
    using SystemType = std::vector<int>;

    auto pushBackGen = Arbitrary<int>().transform<ActionFor<SystemType>>([](const int value) {
        return [value](SystemType& system) {
            std::cout << "PushBack(" << value << ")" << std::endl;
            auto size = system.size();
            system.push_back(value);
            PROP_ASSERT(system.size() == size + 1);
            return true;
        };
    });

    auto popBackGen = actionFor<SystemType>([](SystemType& system) {
        std::cout << "PopBack" << std::endl;
        auto size = system.size();
        if (system.empty())
            return true;
        system.pop_back();
        PROP_ASSERT(system.size() == size - 1);
        return true;
    });

    auto clearGen = actionFor<SystemType>([](SystemType& system) {
        std::cout << "Clear" << std::endl;
        system.clear();
        PROP_ASSERT(system.size() == 0);
        return true;
    });

    auto prop = statefulProperty2<SystemType>(pushBackGen, popBackGen, clearGen);
    prop.forAll();
}
