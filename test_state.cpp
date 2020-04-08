#include "testing/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "testing/Random.hpp"
#include <chrono>
#include <iostream>

using namespace PropertyBasedTesting;

class StateTestCase : public ::testing::Test {
};


struct Action {
    virtual ~Action() {}
    virtual bool precondition() {
        return true;
    }

    virtual bool run() {
        return true;
    };
};

struct PushBack : public Action {
    virtual bool run() {
        std::cout << "PushBack" << std::endl;
        return true;
    }
};

struct Clear : public Action {
    virtual bool run() {
        std::cout << "Clear" << std::endl;
        return true;
    }
};

struct Remove : public Action {
    virtual bool run() {
        std::cout << "Remove" << std::endl;
        return true;
    }
};


//FIXME:

// template <typename T, typename... ARGS>
// Shrinkable<std::shared_ptr<T>> action(ARGS&&...args) {
//     return make_shrinkable<std::shared_ptr<T>>(std::make_shared<T>(std::move(args)...));
// }

// TEST(StateTest, States) {

// // (int, int) ->

//    auto actionGen = oneOf<std::shared_ptr<Action>>(
//         [](Random& rand) {
//             return action<PushBack>();
//         },
//         [](Random& rand) {
//             return action<Remove>();
//         },
//         [](Random& rand) {
//             return action<Clear>();
//         }
//     );

//     int64_t seed = getCurrentTime();
//     Random rand(seed);

//     for(int i = 0; i < 100; i++) {
//         auto action = actionGen(rand);
//         if(action->precondition())
//             action->run();
//     }

// }
