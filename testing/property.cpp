#include "testing/property.hpp"
#include "testing/assert.hpp"
#include <exception>

namespace PropertyBasedTesting {

PropertyBase::PropertyBase() : seed(getCurrentTime()) {
}

bool PropertyBase::check() {
    Random rand(seed);
    std::cout << "rand seed: " << seed << std::endl;
    try {
        // TODO: configurable runs
        for(int i = 0; i < 100; i++) {
            bool pass = true;
            do {
                pass = true;
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
    } catch(const PropertyFailedBase& e) {
        // shrink
        handleShrink(e);
        return false;
    } catch(const std::exception& e) {
        // skip shrinking?
        std::cerr << "std::exception occurred: " << e.what() << std::endl;
        std::cerr << "    seed: " << seed << std::endl;
        return false;
    }
    return true;
}

} // namespace
