#ifndef COMMON_HPP_INCLUDED
#define COMMON_HPP_INCLUDED

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

class NetworkError : public std::exception {
public:
    const char * what() const noexcept { return "network error."; }
};

class EOFException : public std::exception {
public:
    const char * what() const noexcept { return "eof exception."; }
};

class ConnectionRefused : public std::exception {
public:
    const char * what() const noexcept { return "connection refused."; }
};

void bomb(int code);
std::vector<std::string> tokenize(const std::string &line, const char delim = ' ');
void queryServer(const std::vector<std::string> &buffer);
void endConnection();

extern std::string SERVER;
extern bool SCORE_HITS;
extern uint16_t PORT;

#ifdef _WIN32
#include "win32.hpp"
#else
#include "unix.hpp"
#endif

#endif // COMMON_HPP_INCLUDED
