#include "testing/proptest.hpp"
#include "googletest/googletest/include/gtest/gtest.h"
#include "googletest/googlemock/include/gmock/gmock.h"
#include "testing/Random.hpp"
#include <chrono>
#include <iostream>
#include <memory>

using namespace PropertyBasedTesting;

class StateTestCase : public ::testing::Test {
};

struct EmptyModel {
    static EmptyModel value;
};

template <typename SYSTEM, typename MODEL>
struct Action {
    using SystemType = SYSTEM;
    using ModelType = MODEL;

    virtual ~Action() {}
    virtual bool precondition(const SYSTEM& system, const MODEL& model) {
        return precondition(system);
    }

    virtual bool precondition(const SYSTEM& system) {
        return true;
    }

    virtual bool run(SYSTEM& system, MODEL& model) {
        return run(system);
    }

    virtual bool run(SYSTEM& system) {
        throw std::runtime_error("attempt to call undefined run");
    };
};

template <typename SYSTEM>
struct ActionWithoutModel : public Action<SYSTEM, EmptyModel>{
    virtual ~ActionWithoutModel() {}
};


template <typename ActionType, typename... GENS>
std::function<Shrinkable<std::vector<std::shared_ptr<ActionType>>>(Random&)> actions(GENS&&... gens) {
    auto actionGen = oneOf<std::shared_ptr<ActionType>>(std::forward<GENS>(gens)...);
    auto actionVecGen = Arbitrary<std::vector<std::shared_ptr<ActionType>>>(actionGen);
    return actionVecGen;
}

template <typename ActionType, typename InitialGen, typename ActionsGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, ActionsGen&& actionsGen) {
    using SystemType = typename ActionType::SystemType;

    return property([](SystemType obj, std::vector<std::shared_ptr<ActionType>> actions) {
        for(auto action : actions) {
            if(action->precondition(obj))
                action->run(obj);
        }
        return true;
    }, std::forward<InitialGen>(initialGen), std::forward<ActionsGen>(actionsGen));
}

template <typename ActionType, typename InitialGen, typename ModelFactory, typename ActionsGen>
decltype(auto) statefulProperty(InitialGen&& initialGen, ModelFactory&& modelFactory, ActionsGen&& actionsGen) {
    using ModelType = typename ActionType::ModelType;
    using SystemType = typename ActionType::SystemType;
    using ModelFactoryFunction = std::function<ModelType(SystemType&)>;
    std::shared_ptr<ModelFactoryFunction> modelFactoryPtr = std::make_shared<ModelFactoryFunction>(std::forward<ModelFactory>(modelFactory));
    return property([modelFactoryPtr](SystemType obj, std::vector<std::shared_ptr<ActionType>> actions) {
        auto model = (*modelFactoryPtr)(obj);
        for(auto action : actions) {
            if(action->precondition(obj, model))
                action->run(obj, model);
        }
        return true;
    }, std::forward<InitialGen>(initialGen), std::forward<ActionsGen>(actionsGen));
}


struct VectorAction : public ActionWithoutModel<std::vector<int>> {
};

struct PushBack : public VectorAction {
    PushBack(int value) : value(value) {
    }

    virtual bool run(std::vector<int>& system) {
        std::cout << "PushBack(" << value << ")" << std::endl;
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


TEST(StateTest, States) {

    auto actionsGen = actions<VectorAction>(
        transform<int, std::shared_ptr<VectorAction>>(Arbitrary<int>(), [](const int& value) {
            return std::make_shared<PushBack>(value);
        }),
        just<std::shared_ptr<VectorAction>>([]() {
            return std::make_shared<PopBack>();
        }),
        just<std::shared_ptr<VectorAction>>([]() {
            return std::make_shared<Clear>();
        })
    );

    auto prop = statefulProperty<VectorAction>(Arbitrary<std::vector<int>>(), actionsGen);
    prop.check();
}

struct VectorModel {
    VectorModel(int size) : size(size) {
    }
    int size;
};

struct VectorAction2 : public Action<std::vector<int>, VectorModel> {
};

struct PushBack2 : public VectorAction2 {
    PushBack2(int value) : value(value) {
    }

    virtual bool run(std::vector<int>& system, VectorModel& model) {
        std::cout << "PushBack(" << value << ")" << std::endl;
        auto size = system.size();
        system.push_back(value);
        model.size ++;
        PROP_ASSERT(model.size == system.size(), {});
        return true;
    }

    int value;
};

struct Clear2 : public VectorAction2 {
    virtual bool run(std::vector<int>& system, VectorModel& model) {
        std::cout << "Clear" << std::endl;
        system.clear();
        model.size = 0;
        PROP_ASSERT(model.size == system.size(), {});
        return true;
    }
};

struct PopBack2 : public VectorAction2 {
    virtual bool run(std::vector<int>& system, VectorModel& model) {
        std::cout << "PopBack" << std::endl;
        if(system.empty())
            return true;
        system.pop_back();
        model.size --;
        PROP_ASSERT(model.size == system.size(), {});
        return true;
    }
};

TEST(StateTest, StatesWithModel) {

    auto actionsGen = actions<VectorAction2>(
        transform<int, std::shared_ptr<VectorAction2>>(Arbitrary<int>(), [](const int& value) {
            return std::make_shared<PushBack2>(value);
        }),
        just<std::shared_ptr<VectorAction2>>([]() {
            return std::make_shared<PopBack2>();
        }),
        just<std::shared_ptr<VectorAction2>>([]() {
            return std::make_shared<Clear2>();
        })
    );

    auto prop = statefulProperty<VectorAction2>(Arbitrary<std::vector<int>>(), [](std::vector<int>& sys) {
        return VectorModel(sys.size());
    }, actionsGen);
    prop.check();
}

decltype(auto) dummyProperty() {
    using Type = std::function<int()>;
    std::shared_ptr<Type> modelPtr = std::make_shared<Type>([]() {return 0;});
    return property([modelPtr](int dummy) {
        auto model = *modelPtr;
        PROP_STAT(model() > 2);
        return true;
    });
}

TEST(StateTest, PropertyCapture) {
    auto prop = dummyProperty();
    prop.check();
}