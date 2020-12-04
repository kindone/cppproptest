#include "PropertyContext.hpp"
#include "PropertyBase.hpp"
#include "util/std.hpp"

namespace proptest {

ostream& operator<<(ostream& os, const Failure& f)
{
    auto detail = f.str.str();
    if (detail.empty())
        os << f.condition << " (" << f.filename << ":" << f.lineno << ")";
    else
        os << f.condition << " (" << f.filename << ":" << f.lineno << ") with " << f.str.str();
    return os;
}

PropertyContext::PropertyContext() : lastStreamExists(false), oldContext(PropertyBase::getContext())
{
    PropertyBase::setContext(this);
}

PropertyContext::~PropertyContext()
{
    PropertyBase::setContext(oldContext);
}

void PropertyContext::tag(const char* file, int lineno, string key, string value)
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
            valueMap.insert(pair<string, Tag>(value, Tag(file, lineno, value)));
        }
    } else {
        map<string, Tag> valueMap;
        valueMap.insert(pair<string, Tag>(value, Tag(file, lineno, value)));
        tags.insert(pair<string, map<string, Tag>>(key, valueMap));
    }
}

void PropertyContext::succeed(const char*, int, const char*, const stringstream&)
{
    // DO NOTHING
    lastStreamExists = false;
}

void PropertyContext::fail(const char* filename, int lineno, const char* condition, const stringstream& str)
{
    failures.push_back(Failure(filename, lineno, condition, str));
    lastStreamExists = true;
}

stringstream& PropertyContext::getLastStream()
{
    static stringstream defaultStr;
    if (failures.empty() || !lastStreamExists)
        return defaultStr;

    return failures.back().str;
}

stringstream PropertyContext::flushFailures(int indent)
{
    const auto doIndent = +[](stringstream& str, int indent) {
        for (int i = 0; i < indent; i++)
            str << " ";
    };

    stringstream allFailures;
    auto itr = failures.begin();
    if (itr != failures.end()) {
        // doIndent(allFailures, indent);
        allFailures << *itr++;
    }
    for (; itr != failures.end(); ++itr) {
        allFailures << "," << endl;
        doIndent(allFailures, indent);
        allFailures << *itr;
    }
    failures.clear();
    return allFailures;
}

void PropertyContext::printSummary()
{
    for (auto tagKV : tags) {
        auto& key = tagKV.first;
        auto& valueMap = tagKV.second;
        cout << "  " << key << ": " << endl;
        size_t total = 0;
        for (auto valueKV : valueMap) {
            auto tag = valueKV.second;
            total += tag.count;
        }

        for (auto valueKV : valueMap) {
            auto value = valueKV.first;
            auto tag = valueKV.second;
            cout << "    " << value << ": " << tag.count << "/" << total << " ("
                      << static_cast<double>(tag.count) / total * 100 << "%)" << endl;
        }
    }
}

}  // namespace proptest
