#include "testing/property.hpp"
#include "testing/assert.hpp"
#include <exception>

namespace PropertyBasedTesting {

bool PropertyBase::check() {
    Random rand(seed);
    try {
        // TODO: configurable runs
        for(int i = 0; i < 100; i++) {
            bool pass = true;
            do {
                try {
                    invoke(rand);
                    pass = true;
                }
                catch(const Success&) {
                    pass = true;
                }
                catch(const Discard&) {
                    // silently discard combination
                    pass = false;
                }
            } while(!pass);
        }
    } catch(const AssertFailed& e) {
        // TODO:
        // shrink
        //FIXME
        return false;
    } catch(const std::exception& e) {
        // TODO
    }
    return true;
}

} // namespace
