#pragma once

#include <stdexcept>
#include <vector>
#include <memory>
#include <functional>
#include "../util/function_traits.hpp"
#include "../combinator/transform.hpp"
#include "../combinator/oneof.hpp"
#include "../gen.hpp"
#include "../Shrinkable.hpp"
#include "../Random.hpp"
#include "../GenBase.hpp"

namespace proptest {

template <typename SystemType>
using Action = std::function<bool(SystemType&)>;

template <typename SystemType>
decltype(auto) actionGen(Action<SystemType> func)
{
    return just<Action<SystemType>>(func);
}

template <typename SystemType, typename... GENS>
GenFunction<std::vector<std::function<bool(SystemType&)>>> actions(GENS&&... gens)
{
    auto actionGen = oneOf<std::function<bool(SystemType&)>>(std::forward<GENS>(gens)...);
    auto actionVecGen = Arbi<std::vector<std::function<bool(SystemType&)>>>(actionGen);
    return actionVecGen;
}

template <typename SystemType, typename... GENS>
decltype(auto) actionProperty(GENS&&... gens)
{
    auto actionsGen = actions<SystemType>(std::forward<GENS>(gens)...);

    return property(
        [](SystemType obj, std::vector<std::function<bool(SystemType&)>> actions) {
            for (auto action : actions) {
                PROP_ASSERT(action(obj));
            }
            return true;
        },
        Arbi<SystemType>(), actionsGen);
}

}  // namespace proptest
