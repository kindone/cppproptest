#include "testing/Property.hpp"
#include "testing/assert.hpp"
#include <exception>

namespace PropertyBasedTesting {

PropertyContext* PropertyBase::context = nullptr;

PropertyBase::PropertyBase() : seed(getCurrentTime()) {
}

bool PropertyBase::check() {
    Random rand(seed);
    Random savedRand(seed);
    std::cout << "rand seed: " << seed << std::endl;
    PropertyContext context;
    try {
        // TODO: configurable runs
        for(int i = 0; i < 100; i++) {
            bool pass = true;
            do {
                pass = true;
                try {
                    savedRand = rand;
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
        handleShrink(savedRand, e);
        return false;
    } catch(const std::exception& e) {
        // skip shrinking?
        std::cerr << "std::exception occurred: " << e.what() << std::endl;
        std::cerr << "    seed: " << seed << std::endl;
        return false;
    }

    context.printSummary();
    return true;
}

void PropertyBase::setContext(PropertyContext* ctx) {
    if((context && ctx) || (!context && !ctx))
        throw std::runtime_error("invalid argument or property context state");

    context  = ctx;
}

void PropertyBase::tag(const char* file, const char* lineno, std::string key, std::string value) {
    if(!context)
        throw std::runtime_error("context is not set");

    context->tag(file, lineno, key, value);
}

PropertyContext::PropertyContext() {
    PropertyBase::setContext(this);
}

PropertyContext::~PropertyContext() {
    PropertyBase::setContext(nullptr);
}

void PropertyContext::tag(const char* file, const char* lineno, std::string key, std::string value) {
}

void PropertyContext::printSummary() {

}

} // namespace
