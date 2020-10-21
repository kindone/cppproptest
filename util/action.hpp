#pragma once

#include <functional>
#include <string>
#include <iostream>
#include <memory>

namespace proptest {
namespace stateful {


struct EmptyModel
{
};

template <typename ObjectType>
struct SimpleAction {
    using Function = std::function<bool(ObjectType&)>;
    explicit SimpleAction(Function f) : name("Action<?>"), func(std::make_shared<Function>(f)) {}

    SimpleAction(const std::string& _name, Function f) : name(_name), func(std::make_shared<Function>(f)) {}

    bool operator()(ObjectType& obj) const {
        return (*func)(obj);
    }

    friend std::ostream& operator<<(std::ostream& os, const SimpleAction& obj) {
        os << obj.name;
        return os;
    }

    std::string name;
    std::shared_ptr<Function> func;
};

template <typename ObjectType, typename ModelType>
struct Action {
    using Function = std::function<bool(ObjectType&, ModelType&)>;
    explicit Action(Function f) : name("Action<?>"), func(std::make_shared<Function>(f)) {}

    Action(const std::string& _name, Function f) : name(_name), func(std::make_shared<Function>(f)) {}

    Action(const SimpleAction<ObjectType>& simpleAction) : name(simpleAction.name) {
        func = std::make_shared<Function>([simpleAction](ObjectType& obj, ModelType&) {
            return simpleAction(obj);
        });
    }

    bool operator()(ObjectType& obj, ModelType& model) const {
        return (*func)(obj, model);
    }

    friend std::ostream& operator<<(std::ostream& os, const Action& action) {
        os << action.name;
        return os;
    }

    std::string name;
    std::shared_ptr<Function> func;
};

} // namespace stateful
} // namespace proptest
