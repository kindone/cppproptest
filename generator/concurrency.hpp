#pragma once

#include "stateful.hpp"
#include "../gen.hpp"
#include "../Random.hpp"
#include "../Shrinkable.hpp"
#include "../api.hpp"
#include <memory>
#include <type_traits>

namespace PropertyBasedTesting {

template <typename ActionType>
struct ConcurrentActions
{
    std::vector<std::shared_ptr<ActionType>> front;
    std::vector<std::shared_ptr<ActionType>> rear1;
    std::vector<std::shared_ptr<ActionType>> rear2;
};

template <typename ActionType>
class PROPTEST_API Arbitrary<ConcurrentActions<ActionType>> : public Gen<ConcurrentActions<ActionType>> {
public:
    static size_t defaultMinSize;
    static size_t defaultMaxSize;

    Arbitrary(std::function<Shrinkable<std::shared_ptr<ActionType>>(Random&)> elemGen)
        : elemGen(elemGen), minSize(defaultMinSize), maxSize(defaultMaxSize)
    {
    }

    Shrinkable<ConcurrentActions<ActionType>> operator()(Random& rand)
    {
        // auto vecGen = Arbitrary<std::vector<std::shared_ptr<ActionType>>>(elemGen);
        // auto front = vecGen(rand);
        // auto rear1 = vecGen(rand);
        // auto rear2 = vecGen(rand);
        // 1. generate front, rears vectors
        // 2. shrink rears (move to front 1 by 1)

        using vector_t = std::vector<Shrinkable<std::shared_ptr<ActionType>>>;

        int size = rand.getRandomSize(minSize, maxSize + 1);
        std::shared_ptr<vector_t> shrinkVec = std::make_shared<vector_t>();
        shrinkVec->reserve(size);
        for (int i = 0; i < size; i++)
            shrinkVec->push_back(elemGen(rand));

        // 2. shrink rears (move to front 1 by 1)
        auto shrink = [](const ConcurrentActions& ca) { ca.front; };

        return make_shrinkable<ConcurrentActions<ActionType>>();
    }

    Arbitrary<ConcurrentActions<ActionType>> setMinSize(int size)
    {
        minSize = size;
        return *this;
    }

    Arbitrary<ConcurrentActions<ActionType>> setMaxSize(int size)
    {
        maxSize = size;
        return *this;
    }

    Arbitrary<ConcurrentActions<ActionType>> setSize(int size)
    {
        minSize = size;
        maxSize = size;
        return *this;
    }

    // FIXME: turn to shared_ptr
    std::function<Shrinkable<std::shared_ptr<ActionType>>(Random&)> elemGen;
    int minSize;
    int maxSize;
};

template <typename ActionType>
size_t Arbitrary<ConcurrentActions<ActionType>>::defaultMinSize = 1;
template <typename ActionType>
size_t Arbitrary<ConcurrentActions<ActionType>>::defaultMaxSize = 200;

template <typename ActionType, typename InitialGen, typename ModelFactory, typename ActionsGen>
decltype(auto) concurrentProperty(InitialGen&& initialGen, ModelFactory&& modelFactory, ActionsGen&& actionsGen)
{
    using ModelType = typename ActionType::ModelType;
    using SystemType = typename ActionType::SystemType;
    using ModelFactoryFunction = std::function<ModelType(SystemType&)>;
    std::shared_ptr<ModelFactoryFunction> modelFactoryPtr =
        std::make_shared<ModelFactoryFunction>(std::forward<ModelFactory>(modelFactory));

    return property(
        [modelFactoryPtr](SystemType obj, ConcurrentActions<ActionType> actions) {
            auto model = (*modelFactoryPtr)(obj);
            // execute front in serial
            for (auto action : actions.front) {
                if (action->precondition(obj, model))
                    action->run(obj, model);
            }
            // execute rear in parallel
            // TODO
            return true;
        },
        std::forward<InitialGen>(initialGen), std::forward<ActionsGen>(actionsGen));
}

}  // namespace PropertyBasedTesting