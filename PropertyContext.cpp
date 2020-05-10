#include "PropertyContext.hpp"
#include "Property.hpp"
#include <string>
#include <sstream>
#include <map>
#include <stdexcept>
#include <utility>

namespace PropertyBasedTesting {

std::ostream& operator<<(std::ostream& os, const Failure& f)
{
    auto detail = f.str.str();
    if (detail.empty())
        os << f.condition << " (" << f.filename << ":" << f.lineno << ")";
    else
        os << f.condition << " (" << f.filename << ":" << f.lineno << ") with " << f.str.str();
    return os;
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
        auto& valueMap = itr->second;
        auto valueItr = valueMap.find(value);
        // value already exists
        if (valueItr != valueMap.end()) {
            auto& tag = valueItr->second;
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

void PropertyContext::succeed(const char* filename, int lineno, const char* condition, const std::stringstream& str)
{
    // DO NOTHING
}

void PropertyContext::fail(const char* filename, int lineno, const char* condition, const std::stringstream& str)
{
    failures.push_back(Failure(filename, lineno, condition, str));
}

std::stringstream& PropertyContext::getLastStream()
{
    if (failures.empty())
        throw std::runtime_error("no failures recorded");

    return failures.back().str;
}

std::stringstream PropertyContext::flushFailures()
{
    std::stringstream allFailures;
    for (auto itr = failures.begin(); itr != failures.end(); ++itr) {
        allFailures << *itr << std::endl;
    }
    failures.clear();
    return allFailures;
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
