
# [Stateful Testing](./StatefulTesting.md)

## Style 2: Using Action Classes

### Understanding `Action`

In the second style, you will define actions by defining an `Action` or a `SimpleAction` abstract class with following signatures:

```cpp
template <typename ObjectType, typename ModelType>
struct Action
{
    virtual bool precondition(const ObjectType&, const ModelType&) { ... }

    virtual bool run(ObjectType&, ModelType&) { ... }
};

template <typename ObjectType>
struct SimpleAction
{
    virtual bool precondition(const ObjectType&) { ... }

    virtual bool run(ObjectType&) { ... }
};
```

`ObjectType` is the target stateful object type. In this case, `MyVector` is the `ObjectType` type. `ModelType` is optional structure to hold useful data for validation of the `ObjectType` based on some model, throughout the test sequence.

You would prefer to use `SimpleAction` if you don't need a model structure, and `Action` if you need one.

* `precondition` is called to check if an action in the sequence is applicable to current state. If it's not the action is skipped.
    * Overriding `precondition` is optional and returns `true` by default
* `run` is called to actually apply the state change and perform validations against your model after the state change

```cpp
template <typename ObjectType, typename ModelType>
struct Action
{
    virtual bool precondition(const ObjectType& system, const ModelType&) { ... }

    virtual bool run(ObjectType& system, ModelType&) { ... }
};
```

### Defining Actions

For the listed methods that might change the state of a `MyVector`, we would write `Action`s for each.

```cpp
void push_back(int val);
int pop_back();
int& at(int pos);
void clear();
```

```cpp
struct PushBack : public SimpleAction<MyVector> {
    int val;

    PushBack(int val) : val(val) {
    }

    bool run(MyVector& vector) {
        vector.push_back(val);
    }
};

struct PopBack : public SimpleAction<MyVector> {
    bool precondition(MyVector& vector) {
        return vector.size() > 0;
    }

    bool run(MyVector& vector) {
        vector.pop_back(val);
    }
};

struct SetAt : public SimpleAction<MyVector> {
    int pos;
    int val;

    SetAt(int pos, int val) : pos(pos), val(val) {
    }

    bool precondition(MyVector& vector) {
        return pos < vector.size();
    }

    bool run(MyVector& vector) {
        vector.at(pos) = val;
    }
};

struct Clear : public SimpleAction<MyVector> {
    bool run(MyVector& vector) {
        vector.clear(val);
    }
};
```


### Generating `Action` sequence

With our `Action`s properly defined, we can generate a sequence of `Action`s.
`actionClasses` function is a useful shorthand for `oneOf` generator combinator that is specialized for generating `Action` Sequences.

```cpp
        auto actionListGen = actionListGenOf<SimpleAction<MyVector>>(
        // int -> PushBack(int)
        transform<int, std::shared_ptr<SimpleAction<MyVector>>>(
            Arbi<int>(), [](const int& value) { return std::make_shared<PushBack>(value); }),

        // Popback()
        just<std::shared_ptr<SimpleAction<MyVector>>>([]() { return std::make_shared<PopBack>(); }),

        // (int, int) -> SetAt(int, int)
        transform<int, std::shared_ptr<SimpleAction<MyVector>>>(
            Arbi<std::pair<int,int>>(), [](const std:;pair<int,int>& posAndVal) { return std::make_shared<SetAt>(posAndVal.first, posAndVal.second); }),

        // Clear()
        just<std::shared_ptr<SimpleAction<MyVector>>>([]() { return std::make_shared<Clear>(); })
    );
```

This defines a generator for action sequences that randomly chooses `push_back`, `pop_back`, `at`, and `clear` methods for `MyVector` with arguments.

## Running stateful tests

Finally, we will call `statefulProperty::forAll` to perform generation of action sequences and run the tests.
You should supply generator for initial state of `MyVector` to start with.

```cpp
auto prop = statefulProperty<SimpleAction<MyVector>>(Arbi<MyVector>(), actionListGen)
prop.forAll();
```
