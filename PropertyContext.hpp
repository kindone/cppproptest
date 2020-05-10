#pragma once

#include "api.hpp"
#include <string>
#include <map>
#include <sstream>
#include <list>

namespace PropertyBasedTesting {

struct Tag
{
    Tag(const char* f, int l, const std::string& v) : filename(f), lineno(l), value(v), count(1) {}
    const char* filename;
    const int lineno;
    std::string value;
    size_t count;
};

struct Failure
{
    Failure(const char* f, int l, const char* c, const std::stringstream& s)
        : filename(f), lineno(l), condition(c), str(s.str())
    {
    }
    const char* filename;
    const int lineno;
    const char* condition;
    std::stringstream str;
};

std::ostream& operator<<(std::ostream&, const Failure&);

struct PROPTEST_API PropertyContext
{
    PropertyContext();
    ~PropertyContext();

    void tag(const char* filename, int lineno, std::string key, std::string value);
    void succeed(const char* filename, int lineno, const char* condition, const std::stringstream& str);
    void fail(const char* filename, int lineno, const char* condition, const std::stringstream& str);
    void tag(std::string key, std::string value) { tag("?", -1, key, value); }
    std::stringstream& getLastStream();
    std::stringstream flushFailures();
    void printSummary();

private:
    // key -> (value -> Tag(count, detail))
    std::map<std::string, std::map<std::string, Tag> > tags;
    std::list<Failure> failures;
};

}  // namespace PropertyBasedTesting