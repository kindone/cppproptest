#include "proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "Random.hpp"
#include <chrono>
#include <iostream>
#include <memory>

using namespace PropertyBasedTesting;

// template <typename ActionType, typename... GENS>
// std::function<Shrinkable<std::vector<std::shared_ptr<ActionType>>>(Random&)> actions2(GENS&&... gens)
// {
//     auto actionGen = oneOf<std::shared_ptr<ActionType>>(util::toSharedPtrGen<ActionType>(std::forward<GENS>(gens))...);
//     auto actionVecGen = Arbitrary<std::vector<std::shared_ptr<ActionType>>>(actionGen);
//     return actionVecGen;
// }

// template <typename SystemType, typename InitialGen, typename ActionsGen>
// decltype(auto) statefulProperty2(InitialGen&& initialGen, std::function< Shrinkable<std::vector<std::function<bool(SystemType&)>>>(Random&)> && actionsGen)
// {
//     return property(
//         [](SystemType obj, std::vector<std::shared_ptr<ActionType>> actions) {
//             for (auto action : actions) {
//                 if (action->precondition(obj))
//                     PROP_ASSERT(action->run(obj));
//             }
//             return true;
//         },
//         std::forward<InitialGen>(initialGen), std::forward<ActionsGen>(actionsGen));
// }

TEST(StateTest, States2)
{
    using SystemType = std::vector<int>;

    auto pushBackGen = transform<int, std::function<bool(SystemType&)>>(Arbitrary<int>(),  [](const int value) {
        return [value](SystemType& system)
        {
            std::cout << "PushBack(" << value << ")" << std::endl;
            auto size = system.size();
            system.push_back(value);
            PROP_ASSERT(system.size() == size + 1);
            return true;
        };
    });

    auto popBackGen = [](Random&) {
        return make_shrinkable<std::function<bool(SystemType&)>>([](SystemType& system) {
            std::cout << "PopBack" << std::endl;
            auto size = system.size();
            if (system.empty())
                return true;
            system.pop_back();
            PROP_ASSERT(system.size() == size - 1);
            return true;
        });
    };

    auto clearGen = [](Random&) {
        return make_shrinkable<std::function<bool(SystemType&)>>([](SystemType& system) {
            std::cout << "Clear" << std::endl;
            system.clear();
            PROP_ASSERT(system.size() == 0);
            return true;
        });
    };
    auto actionGen = oneOf<std::function<bool(SystemType&)>>(pushBackGen, popBackGen, clearGen);
    auto actionsGen = Arbitrary<std::vector<std::function<bool(SystemType&)>>>(actionGen);

    property(
        [](SystemType obj, std::vector<std::function<bool(SystemType&)>> actions) {
            for (auto action : actions) {
                PROP_ASSERT(action(obj));
            }
            return true;
        },
        Arbitrary<SystemType>(), actionsGen);
}
