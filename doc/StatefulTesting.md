# Stateful testing

While property-based testing suits well with functions and stateless objects, it's possible to test various state changes with ease. The idea of stateful testing with `cppproptest` is to define and utilize generators for state changes.

* Define `action`s that each represents unit of state change - e.g. calling `add` method with arguments
* Define a generator for `action` type that can build a seqeunce of actions and provide arguments to the selected actions
* Run the action sequences generated as in usual property tests

### Understanding `Action`

Say, you are to write stateful test for your `MyVector`, which is a linear container for integers with random-access support.

```cpp
class MyVector {
    void push_back(int val);
    int pop_back();
    int size();
    int& at(int pos);
    void clear();
};
```

You first need to define actions for each state change. This can be done by extending `PropertyBasedTesting::Action` or `PropertyBasedTesting::ActionWithoutModel` abstract class with following signature:


```cpp
template <typename SYSTEM>
struct ActionWithoutModel
{
    virtual bool precondition(const SYSTEM&) { ... }

    virtual bool run(SYSTEM&) { ... }
};
```

`SYSTEM` is the target stateful object type. In this case, `MyVector` is the `SYSTEM` type. `MODEL` is an optional structure to hold useful data for validation of the `SYSTEM` based on some model, throughout the test sequence.

You would prefer `ActionWithoutModel` if you don't need a model structure and `Action` if you need one.

* `precondition` is called to check if an action in the sequence is applicable to current state. If it's not the action is skipped.
    * Overriding `precondition` is optional and returns `true` by default
* `run` is called to actually apply the state change and perform validations against your model after the state change
    
```cpp
template <typename SYSTEM, typename MODEL>
struct Action
{
    virtual bool precondition(const SYSTEM& system, const MODEL&) { ... }
 
    virtual bool run(SYSTEM& system, MODEL&) { ... }
};
```

### Defining `Action`s

For the listed methods that might change the state of a `MyVector`, we would write `Action`s for each.

```cpp
void push_back(int val);
int pop_back();
int& at(int pos);
void clear();
```

```cpp
struct PushBack : public ActionWithoutModel<MyVector> {
    int val;
    
    PushBack(int val) : val(val) {
    }
    
    bool run(MyVector& vector) {
        vector.push_back(val);
    }
};

struct PopBack : public ActionWithoutModel<MyVector> {    
    bool precondition(MyVector& vector) {
        return vector.size() > 0;
    }
    
    bool run(MyVector& vector) {
        vector.pop_back(val);
    }
};

struct SetAt : public ActionWithoutModel<MyVector> {
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

struct Clear : public ActionWithoutModel<MyVector> {
    bool run(MyVector& vector) {
        vector.clear(val);
    }
};
```


### Generating `Action` sequence

With our `Action`s properly defined, we can generate the sequence of `Action`s.
`actions` function is a useful shorthand for `oneOf` generator combinator that is specialized for generating `Action` Sequences.

```cpp
auto actionSeqGen = actions<ActionWithoutModel<MyVector>>(
        // int -> PushBack(int)
        transform<int, std::shared_ptr<ActionWithoutModel<MyVector>>>(
            Arbitrary<int>(), [](const int& value) { return std::make_shared<PushBack>(value); }),
            
        // Popback()
        just<std::shared_ptr<ActionWithoutModel<MyVector>>>([]() { return std::make_shared<PopBack>(); }),
        
        // (int, int) -> SetAt(int, int)
        transform<int, std::shared_ptr<ActionWithoutModel<MyVector>>>(
            Arbitrary<std::pair<int,int>>(), [](const std:;pair<int,int>& posAndVal) { return std::make_shared<SetAt>(posAndVal.first, posAndVal.second); }),
            
        // Clear()
        just<std::shared_ptr<ActionWithoutModel<MyVector>>>([]() { return std::make_shared<Clear>(); })
    );
```

This defines a generator for action sequences that randomly chooses `push_back`, `pop_back`, `at`, and `clear` methods for `MyVector` with arguments.

### Running stateful tests

Finally, we will call `statefulProperty::forAll` to perform generation of action sequences and run the tests.
You should supply generator for initial state of `MyVector` to start with.

```cpp
auto prop = statefulProperty<ActionWithoutModel<MyVector>>(Arbitrary<MyVector>(), actionSeqGen)
prop.forAll();
```
