#include "PropertyBase.hpp"
#include "assert.hpp"
#include "util/tuple.hpp"
#include <exception>
#include <utility>

namespace proptest {

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

bool PropertyBase::invoke(Random&)
{
    return true;
}

std::stringstream& PropertyBase::getLastStream()
{
    if (!context)
        throw std::runtime_error("context is not set");

    return context->getLastStream();
}

}  // namespace proptest
