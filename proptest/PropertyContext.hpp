#pragma once

#include "api.hpp"
#include "util/std.hpp"

namespace proptest {

struct Tag
{
    Tag(const char* f, int l, const string& v) : filename(f), lineno(l), value(v), count(1) {}
    const char* filename;
    const int lineno;
    string value;
    size_t count;
};

struct Failure
{
    Failure(const char* f, int l, const char* c, const stringstream& s)
        : filename(f), lineno(l), condition(c), str(s.str())
    {
    }
    const char* filename;
    const int lineno;
    const char* condition;
    stringstream str;
};

namespace util {
PROPTEST_API uint64_t getGlobalSeed();
}

ostream& operator<<(ostream&, const Failure&);

struct PROPTEST_API PropertyContext
{
    PropertyContext();
    ~PropertyContext();

    void tag(const char* filename, int lineno, string key, string value);
    void succeed(const char* filename, int lineno, const char* condition, const stringstream& str);
    void fail(const char* filename, int lineno, const char* condition, const stringstream& str);
    void tag(string key, string value) { tag("?", -1, key, value); }
    stringstream& getLastStream();
    stringstream flushFailures(int indent = 0);
    void printSummary();
    bool hasFailures() const { return !failures.empty(); }

private:
    // key -> (value -> Tag(count, detail))
    map<string, map<string, Tag> > tags;
    list<Failure> failures;
    bool lastStreamExists;

    PropertyContext* oldContext;
};

}  // namespace proptest
