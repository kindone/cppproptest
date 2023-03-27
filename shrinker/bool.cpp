#include "bool.hpp"

namespace proptest {

Shrinkable<bool> shrinkBool(bool value) {
    if (value) {
        return make_shrinkable<bool>(value).with(
            +[]() { return Stream::one<Shrinkable<bool>>(make_shrinkable<bool>(false)); });
    } else {
        return make_shrinkable<bool>(value);
    }
}

}
