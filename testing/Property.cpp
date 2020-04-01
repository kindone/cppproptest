#include "testing/Property.hpp"
#include "testing/assert.hpp"
#include <exception>
#include <utility>

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

void PropertyBase::tag(const char* file, int lineno, std::string key, std::string value) {
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

void PropertyContext::tag(const char* file, int lineno, std::string key, std::string value) {
    auto itr = tags.find(key);
    // key already exists
    if(itr != tags.end()) {
        auto valueMap = itr->second;
        auto valueItr = valueMap.find(value);
        // value already exists
        if(valueItr != valueMap.end()) {
            auto tag = valueItr->second;
            tag.count ++;
        }
        else {
            std::map<std::string, Tag>& valueMap = itr->second;
            valueMap.insert(std::pair<std::string, Tag>(value, Tag(file, lineno, value)));
        }
    }
    else {
        std::map<std::string, Tag> valueMap;
        valueMap.insert(std::pair<std::string, Tag>(value, Tag(file, lineno, value)));
        tags.insert(std::pair<std::string, std::map<std::string, Tag>>(key, valueMap));
    }
}

void PropertyContext::printSummary() {
    for(auto tagKV : tags) {
        auto& key = tagKV.first;
        auto& valueMap = tagKV.second;
        std::cout << key << ": " << std::endl;
        size_t total = 0;
        for(auto valueKV : valueMap) {
            auto tag = valueKV.second;
            total += tag.count;
        }

        for(auto valueKV : valueMap) {
            auto value = valueKV.first;
            auto tag = valueKV.second;
            std::cout << "  value = " << value << ": " << static_cast<double>(tag.count)/total*100 << "%" << std::endl;
        }
    }
}

} // namespace
