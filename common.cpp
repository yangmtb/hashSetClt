#include "common.hpp"
#include <algorithm>
#include <fstream>
#include <memory>

using std::string;
using std::vector;
using std::find;
using std::remove;
using std::ofstream;
using std::unique_ptr;
using std::make_unique;

vector<string> tokenize(const string &line, const char delim)
{
    auto begin { line.begin() };
    auto end { line.end() };
    vector<string> rv;
    while (begin != line.end()) {
        end = find(begin+1, line.end(), delim);
        rv.emplace_back(string(begin, end));
        begin = end+(end == line.end() ? 0 : 1);
    }
    for (size_t idx = 0; idx < rv.size(); ++idx) {
        rv.at(idx).erase(remove(rv.at(idx).begin(), rv.at(idx).end(), '\r'), rv.at(idx).end());
        rv.at(idx).erase(remove(rv.at(idx).begin(), rv.at(idx).end(), '\n'), rv.at(idx).end());
    }
    rv.erase(remove(rv.begin(), rv.end(), ""), rv.end());
    return rv;
}

void bomb(int code)
{
#ifdef WINDOWS
    WSACleanup();
    ExitProcess(code);
#else
    exit(code);
#endif // WINDOWS
}
