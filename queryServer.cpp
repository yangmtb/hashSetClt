#include "common.hpp"

using std::string;
using std::vector;
using std::ofstream;

namespace {

NetworkSocket sockObj;

class BadHandshake : public std::exception {
public:
    const char * what() const noexcept { return "bad handshake."; }
};

class BadQuery : public std::exception {
public:
    const char * what() const noexcept { return "bad query."; }
};

class MismatchedResultSet : public std::exception {
public:
    const char * what() const noexcept { return "mismatched result set."; }
};

void writeOutput(const vector<string> &buffer, const string &resultLine)
{
    auto tokens { tokenize(resultLine) };
    if (tokens.empty()) {
        std::cout << "empty" << std::endl;
        return;
    }
    //std::cout << "result line:" << resultLine << std::endl;
    if (tokens.at(0) != "OK") {
        throw BadQuery();
    }
    const string &results = tokens.at(1);
    if (buffer.size() != results.size()) {
        throw MismatchedResultSet();
    }
    for (size_t idx = 0; idx < buffer.size(); ++idx) {
        bool hit = results.at(idx) == '1' ? true : false;
        if ((hit && SCORE_HITS) || (!hit && !SCORE_HITS)) {
            std::cout << buffer.at(idx) << std::endl;
        }
    }
    std::cout << std::endl;
}

} // namespace

void endConnection()
{
    try {
        if (sockObj.isConnected()) {
            sockObj.write("bye\r\n");
        }
    } catch (std::exception &) {
        // pass : we are closing the connection anyway.
    }
}

void queryServer(const vector<string> &buffer)
{
    if (buffer.empty()) {
        return;
    }
    if (!sockObj.isConnected()) {
        try {
            sockObj.connect(SERVER, PORT);
            sockObj.write("Version\r\n");
            auto tokens { tokenize(sockObj.readLine()) };
            if (tokens.size() == 0 || tokens.at(0) != "OK") {
                throw BadHandshake();
            }
            std::cout << "connect " << tokens.at(0) << std::endl;
        } catch (ConnectionRefused &) {
            std::cerr << "Error: connection refused" << std::endl;
            bomb(-1);
        } catch (BadHandshake &) {
            std::cerr << "Error: server handshake failed." << std::endl;
            bomb(-1);
        }
    }
    try {
        string q { "query" };
        for (size_t idx = 0; idx < buffer.size(); ++idx) {
            auto tokets { tokenize(buffer.at(idx)) };
            q += " " + tokets.at(0);
        }
        q += "\r\n";
        sockObj.write(q);
        //std::cout << "q:" << q << std::endl;
        writeOutput(buffer, sockObj.readLine());
    } catch (BadQuery &) {
        std::cerr << "error: server didn't like our query." << std::endl;
        bomb(-1);
    } catch (NetworkError &) {
        std::cerr << "error: network failure." << std::endl;
        bomb(-1);
    } catch (MismatchedResultSet &) {
        std::cerr << "error: mismatched result set." << std::endl;
        bomb(-1);
    } catch (std::exception &) {
        std::cerr << "error: unknown error WTF...." << std::endl;
        bomb(-1);
    }
}
