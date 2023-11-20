#include "proptest/shrinker/bool.hpp"

namespace proptest {

Shrinkable<bool> shrinkBool(bool value) {
    if (value) {
        return make_shrinkable<bool>(value).with(
            +[]() { return Stream::one<ShrinkableAny>(make_shrinkable<bool>(false)); });
    } else {
        return make_shrinkable<bool>(value);
    }
}

}
