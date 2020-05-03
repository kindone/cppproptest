#include "Property.hpp"
#include "assert.hpp"
#include "tuple.hpp"
#include <exception>
#include <utility>

namespace PropertyBasedTesting {

namespace util {

uint64_t getGlobalSeed() {
    static const char* env_seed = std::getenv("PROPTEST_SEED");
    if(env_seed) {
       return atoll(env_seed);
    }
    else {
        static uint64_t time = getCurrentTime();
        return time;
    }
}

}


PropertyContext* PropertyBase::context = nullptr;
uint32_t PropertyBase::defaultNumRuns = 100;

PropertyBase::PropertyBase() : seed(util::getGlobalSeed()), numRuns(defaultNumRuns) {}

bool PropertyBase::check()
{
    Random rand(seed);
    Random savedRand(seed);
    std::cout << "random seed: " << seed << std::endl;
    PropertyContext context;
    int i = 0;
    try {
        // TODO: configurable runs
        for (; i < numRuns; i++) {
            bool pass = true;
            do {
                pass = true;
                try {
                    savedRand = rand;
                    invoke(rand);
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
        // shrink
        handleShrink(savedRand /*, e*/);
        return false;
    } catch (const std::exception& e) {
        // skip shrinking?
        std::cerr << "Falsifiable, after " << (i + 1) << " tests - std::exception occurred: " << e.what() << std::endl;
        handleShrink(savedRand /*, e*/);
        return false;
    }

    std::cout << "OK, passed " << numRuns << " tests" << std::endl;
    context.printSummary();
    return true;
}

void PropertyBase::setContext(PropertyContext* ctx)
{
    if ((context && ctx) || (!context && !ctx))
        throw std::runtime_error("invalid argument or property context state");

    context = ctx;
}

void PropertyBase::tag(const char* file, int lineno, std::string key, std::string value)
{
    if (!context)
        throw std::runtime_error("context is not set");

    context->tag(file, lineno, key, value);
}

PropertyContext::PropertyContext()
{
    PropertyBase::setContext(this);
}

PropertyContext::~PropertyContext()
{
    PropertyBase::setContext(nullptr);
}

void PropertyContext::tag(const char* file, int lineno, std::string key, std::string value)
{
    auto itr = tags.find(key);
    // key already exists
    if (itr != tags.end()) {
        auto valueMap = itr->second;
        auto valueItr = valueMap.find(value);
        // value already exists
        if (valueItr != valueMap.end()) {
            auto tag = valueItr->second;
            tag.count++;
        } else {
            std::map<std::string, Tag>& valueMap = itr->second;
            valueMap.insert(std::pair<std::string, Tag>(value, Tag(file, lineno, value)));
        }
    } else {
        std::map<std::string, Tag> valueMap;
        valueMap.insert(std::pair<std::string, Tag>(value, Tag(file, lineno, value)));
        tags.insert(std::pair<std::string, std::map<std::string, Tag>>(key, valueMap));
    }
}

void PropertyContext::printSummary()
{
    for (auto tagKV : tags) {
        auto& key = tagKV.first;
        auto& valueMap = tagKV.second;
        std::cout << "  " << key << ": " << std::endl;
        size_t total = 0;
        for (auto valueKV : valueMap) {
            auto tag = valueKV.second;
            total += tag.count;
        }

        for (auto valueKV : valueMap) {
            auto value = valueKV.first;
            auto tag = valueKV.second;
            std::cout << "    " << value << ": " << static_cast<double>(tag.count) / total * 100 << "%" << std::endl;
        }
    }
}

}  // namespace PropertyBasedTesting
