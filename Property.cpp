#include "PropertyBase.hpp"
#include "assert.hpp"
#include "util/tuple.hpp"
#include "util/std.hpp"

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

}  // namespace utilr

PropertyContext* PropertyBase::context = nullptr;
uint32_t PropertyBase::defaultNumRuns = 1000;
uint32_t PropertyBase::defaultMaxDurationMs = 0;

void PropertyBase::setContext(PropertyContext* ctx)
{
    context = ctx;
}

void PropertyBase::tag(const char* file, int lineno, string key, string value)
{
    if (!context)
        throw runtime_error("context is not set");

    context->tag(file, lineno, key, value);
}

void PropertyBase::succeed(const char* file, int lineno, const char* condition, const stringstream& str)
{
    if (!context)
        throw runtime_error("context is not set");

    context->succeed(file, lineno, condition, str);
}

void PropertyBase::fail(const char* file, int lineno, const char* condition, const stringstream& str)
{
    if (!context)
        throw runtime_error("context is not set");

    context->fail(file, lineno, condition, str);
}

bool PropertyBase::invoke(Random&)
{
    return true;
}

stringstream& PropertyBase::getLastStream()
{
    if (!context)
        throw runtime_error("context is not set");

    return context->getLastStream();
}

}  // namespace proptest
