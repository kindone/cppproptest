#include "proptest/api.hpp"
#include "proptest/util/std.hpp"
#include "proptest/util/any.hpp"
#include "proptest/Stream.hpp"
#include "proptest/Shrinkable.hpp"
#include "proptest/assert.hpp"
#include "proptest/util/tuple.hpp"

#include "proptest/util/utf8string.hpp"
#include "proptest/util/utf16string.hpp"
#include "proptest/util/cesu8string.hpp"
#include "proptest/util/unicode.hpp"
#include "proptest/util/nullable.hpp"
#include "proptest/util/action.hpp"
#include "proptest/util/printing.hpp"

#include "proptest/gen.hpp"
#include "proptest/util/function_traits.hpp"
#include "proptest/util/tuple.hpp"

#include "proptest/util/invokeWithGenTuple.hpp"
#include "proptest/util/invokeWithArgs.hpp"
#include "proptest/util/createGenTuple.hpp"
#include "proptest/generator/util.hpp"
#include "proptest/PropertyContext.hpp"
#include "proptest/PropertyBase.hpp"
#include "proptest/Property.hpp"

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
