#include "testing/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "testing/Random.hpp"
#include <chrono>
#include <iostream>

using namespace PropertyBasedTesting;

class StateTestCase : public ::testing::Test {
};

struct EmptyModel {
    static EmptyModel value;
};

template <typename SYSTEM, typename MODEL>
struct Action {
    virtual ~Action() {}
    virtual bool precondition(const SYSTEM& system, const MODEL& model) {
        return true;
    }

    virtual bool run(SYSTEM& system, MODEL& model) {
        return true;
    };
};

template <typename SYSTEM>
struct ActionWithoutModel {
    virtual ~ActionWithoutModel() {}
    virtual bool precondition(const SYSTEM& system) {
        return true;
    }

    virtual bool run(SYSTEM& system) {
        return true;
    };
};

struct VectorAction : public ActionWithoutModel<std::vector<int>> {
};

struct PushBack : public VectorAction {
    PushBack(int value) : value(value) {
    }

    virtual bool run(std::vector<int>& system) {
        std::cout << "PushBack" << std::endl;
        auto size = system.size();
        system.push_back(value);
        PROP_ASSERT(system.size() == size+1, {});
        return true;
    }

    int value;
};

struct Clear : public VectorAction {
    virtual bool run(std::vector<int>& system) {
        std::cout << "Clear" << std::endl;
        system.clear();
        PROP_ASSERT(system.size() == 0, {});
        return true;
    }
};

struct PopBack : public VectorAction {
    virtual bool run(std::vector<int>& system) {
        std::cout << "PopBack" << std::endl;
        auto size = system.size();
        if(system.empty())
            return true;
        system.pop_back();
        PROP_ASSERT(system.size() == size-1, {});
        return true;
    }
};

template <typename T, typename... ARGS>
Shrinkable<std::shared_ptr<VectorAction>> action(ARGS&&...args) {
    return make_shrinkable<std::shared_ptr<VectorAction>>(std::make_shared<T>(std::forward<ARGS>(args)...));
}

TEST(StateTest, States) {

// (int, int) ->

    auto actionGen = oneOf<std::shared_ptr<VectorAction>>(
        transform<int, std::shared_ptr<VectorAction>>(Arbitrary<int>(), [](const int& value) -> std::shared_ptr<VectorAction> {
            return std::make_shared<PushBack>(value);
        }),
        [](Random& rand) {
            return action<PopBack>();
        },
        [](Random& rand) {
            return action<Clear>();
        }
    );

    int64_t seed = getCurrentTime();
    Random rand(seed);

    std::vector<int> initialVec;

    for(int i = 0; i < 100; i++) {
        auto action = actionGen(rand).get();
        if(action->precondition(initialVec))
            action->run(initialVec);
    }

}
