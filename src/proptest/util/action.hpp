#pragma once

#include "proptest/util/std.hpp"

namespace proptest {
namespace stateful {


struct EmptyModel
{
};

template <typename ObjectType>
struct SimpleAction {
    using Function = function<void(ObjectType&)>;
    explicit SimpleAction(Function f) : name("Action<?>"), func(util::make_shared<Function>(f)) {}

    SimpleAction(const string& _name, Function f) : name(_name), func(util::make_shared<Function>(f)) {}

    void operator()(ObjectType& obj) const {
        (*func)(obj);
    }

    friend ostream& operator<<(ostream& os, const SimpleAction& obj) {
        os << obj.name;
        return os;
    }

    string name;
    shared_ptr<Function> func;
};

template <typename ObjectType, typename ModelType>
struct Action {
    using Function = function<void(ObjectType&, ModelType&)>;
    explicit Action(Function f) : name("Action<?>"), func(util::make_shared<Function>(f)) {}

    Action(const string& _name, Function f) : name(_name), func(util::make_shared<Function>(f)) {}

    Action(const SimpleAction<ObjectType>& simpleAction) : name(simpleAction.name) {
        func = util::make_shared<Function>([simpleAction](ObjectType& obj, ModelType&) {
            return simpleAction(obj);
        });
    }

    void operator()(ObjectType& obj, ModelType& model) const {
        (*func)(obj, model);
    }

    friend ostream& operator<<(ostream& os, const Action& action) {
        os << action.name;
        return os;
    }

    string name;
    shared_ptr<Function> func;
};

} // namespace stateful
} // namespace proptest
