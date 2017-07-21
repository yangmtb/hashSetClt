#include <iostream>
#include "common.hpp"
#include <array>
#include <cctype>
#include <regex>

using std::array;
using std::vector;
using std::string;
using std::find;
using std::fill;
using std::remove;
using std::ofstream;
using std::cin;
using std::unique_ptr;
using std::cout;
using std::regex_search;
using std::regex;
using std::transform;
using std::endl;

string SERVER { "192.168.41.137" };
bool SCORE_HITS { false };
uint16_t PORT { 9998 };

int main()
{
#ifdef WINDOWS
    WSAData wsad;
    if (0 != WSAStartup(MAKEWORD(2, 0), &wsad)) {
        std::cerr << "error: could not initialize winsock." << endl;
        bomb(-1);
    }
#endif // WINDOWS
    cout << "input 'query' to query" << endl;
    SCORE_HITS = true;
    vector<string> buffer;
    //buffer.push_back(string { "QUERY" });
    regex validLine { "^[A-Fa-f0-9]{32}", std::regex_constants::icase | std::regex_constants::optimize };
    try {
        string line;
        while (cin) {
            getline(cin, line);
            if (line == "query") {
                break;
            }
            transform(line.begin(), line.end(), line.begin(), ::toupper);
            if (regex_search(line, validLine)) {
                buffer.push_back(string(line.begin(), line.begin()+32));
                if (buffer.size() > 4096) {
                    queryServer(buffer);
                    buffer.clear();
                }
                //break;
            }
        }
    } catch (EOFException &) {
        std::cerr << "???" << endl;
        // pass : this is entirely expected. Uh, well, maybe. it should actually be removed , i think..........
    }
    if (buffer.size()) {
        queryServer(buffer);
        buffer.clear();
    }
    endConnection();
    return 0;
}
