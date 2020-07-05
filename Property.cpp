#include "PropertyBase.hpp"
#include "assert.hpp"
#include "util/tuple.hpp"
#include <exception>
#include <utility>

namespace pbt {

namespace util {

uint64_t getGlobalSeed()
{
    static const char* env_seed = std::getenv("PROPTEST_SEED");
    if (env_seed) {
        return atoll(env_seed);
    } else {
        static uint64_t time = getCurrentTime();
        return time;
    }
}

}  // namespace util

PropertyContext* PropertyBase::context = nullptr;
uint32_t PropertyBase::defaultNumRuns = 1000;

PropertyBase::PropertyBase() : seed(util::getGlobalSeed()), numRuns(defaultNumRuns) {}

bool PropertyBase::forAll()
{
    Random rand(seed);
    Random savedRand(seed);
    std::cout << "random seed: " << seed << std::endl;
    PropertyContext ctx;
    int i = 0;
    try {
        for (; i < numRuns; i++) {
            bool pass = true;
            do {
                pass = true;
                try {
                    savedRand = rand;
                    bool result = invoke(rand);
                    std::stringstream failures = ctx.flushFailures();
                    if (failures.rdbuf()->in_avail()) {
                        std::cerr << "Falsifiable, after " << (i + 1) << " tests: ";
                        std::cerr << failures.str();
                        handleShrink(savedRand /*, e*/);
                        return false;
                    } else if (!result) {
                        std::cerr << "Falsifiable, after " << (i + 1) << " tests" << std::endl;
                        handleShrink(savedRand /*, e*/);
                        return false;
                    }
                    pass = true;
                } catch (const Success&) {
                    pass = true;
                } catch (const Discard&) {
                    // silently discard combination
                    pass = false;
                }
            } while (!pass);
        }
    } catch (const PropertyFailedBase& e) {
        std::cerr << "Falsifiable, after " << (i + 1) << " tests: " << e.what() << " (" << e.filename << ":" << e.lineno
                  << ")" << std::endl;
        // std::cerr << ctx.flushFailures(2).str();
        // shrink
        handleShrink(savedRand /*, e*/);
        return false;
    } catch (const std::exception& e) {
        // skip shrinking?
        std::cerr << "Falsifiable, after " << (i + 1) << " tests - unhandled exception thrown: " << e.what()
                  << std::endl;
        // std::cerr << ctx.flushFailures(2).str();
        handleShrink(savedRand /*, e*/);
        return false;
    }

    std::cout << "OK, passed " << numRuns << " tests" << std::endl;
    ctx.printSummary();
    return true;
}

void PropertyBase::setContext(PropertyContext* ctx)
{
    context = ctx;
}

void PropertyBase::tag(const char* file, int lineno, std::string key, std::string value)
{
    if (!context)
        throw std::runtime_error("context is not set");

    context->tag(file, lineno, key, value);
}

void PropertyBase::succeed(const char* file, int lineno, const char* condition, const std::stringstream& str)
{
    if (!context)
        throw std::runtime_error("context is not set");

    context->succeed(file, lineno, condition, str);
}

void PropertyBase::fail(const char* file, int lineno, const char* condition, const std::stringstream& str)
{
    if (!context)
        throw std::runtime_error("context is not set");

    context->fail(file, lineno, condition, str);
}

std::stringstream& PropertyBase::getLastStream()
{
    if (!context)
        throw std::runtime_error("context is not set");

    return context->getLastStream();
}

}  // namespace pbt
