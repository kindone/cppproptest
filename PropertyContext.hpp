#pragma once

#include "api.hpp"
#include <string>
#include <map>

namespace PropertyBasedTesting {

struct Tag
{
    Tag(const char* f, int l, const std::string& v) : filename(f), lineno(l), value(v), count(1) {}
    const char* filename;
    const int lineno;
    std::string value;
    size_t count;
};

struct PROPTEST_API PropertyContext
{
    PropertyContext();
    ~PropertyContext();

    void tag(const char* filename, int lineno, std::string key, std::string value);
    void tag(std::string key, std::string value) { tag("?", -1, key, value); }
    void printSummary();

private:
    std::map<std::string, std::map<std::string, Tag> > tags;
};

}  // namespace PropertyBasedTesting