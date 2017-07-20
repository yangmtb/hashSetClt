#ifndef UNIX_HPP_INCLUDED
#define UNIX_HPP_INCLUDED

#include <netdb.h>
#include <poll.h>
#include <sys/errno.h>
#include <unistd.h>
#include <algorithm>

#ifdef __APPLE__
#endif // __APPLE__

#ifdef __linux__
#include <cstring>
#include <sys/socket.h>
#endif // __linux__

#ifdef __FreeBSD__
#include <netinet/in.h>
#include <sys/socket.h>
#endif // __FreeBSD__

class NetworkSocket {
public:
    NetworkSocket()
        : sock { -1 }
        , buffer { "" }
    {
    }

    void connect(std::string host, uint16_t port)
    {
        struct sockaddr_in servAddr;
        struct hostent *server = gethostbyname(host.c_str());

        if (nullptr == server || 0 > (sock = socket(AF_INET, SOCK_STREAM, 0))) {
            std::cerr << "client socket err" << std::endl;
            throw NetworkError();
        }

        memset(&servAddr, 0, sizeof(servAddr));
        servAddr.sin_family = AF_INET;
        servAddr.sin_port = htons(port);
        memcpy(static_cast<void *>(&servAddr.sin_addr.s_addr),
               static_cast<void *>(server->h_addr),
               static_cast<size_t>(server->h_length));

        if (::connect(sock, reinterpret_cast<sockaddr *>(&servAddr), sizeof(servAddr)) < 0) {
            if (ECONNREFUSED == errno) {
                throw ConnectionRefused();
            } else {
                throw NetworkError();
            }
        }
    }

    bool isConnected() const { return (sock > 0); }

    void disconnect()
    {
        if (sock > 0) {
            close(sock);
        }
        sock = -1;
    }

    ~NetworkSocket() { disconnect(); }

    void write(const std::string &line)
    {
        // this has a wacky edge case if you are sending 2**32 bytes and have a network error. yes, it's a bug.
        if (line.size() != static_cast<size_t>(send(sock, line.c_str(), line.size(), 0))) {
            throw NetworkError();
        }
    }

    void write(const char *buf)
    {
        std::string line(buf);
        write(line);
    }

    std::string readLine()
    {
        pollfd fds = { sock, POLLIN, 0 };
        int pollCode { poll(&fds, 1, 750) };
        std::string rv { "" };
        auto iter { std::find(buffer.begin(), buffer.end(), '\n') };
        //auto iter { std::find_if(buffer.begin(), buffer.end(), [&](auto x) { return x == '\n'; }) };

        while (0 == pollCode) {
            pollCode = poll(&fds, 1, 750);
        }
        if (-1 == pollCode) {
            throw NetworkError();
        } else if (fds.revents & POLLERR || fds.revents & POLLHUP) {
            throw NetworkError();
        } else if (fds.revents & POLLIN) {
            if (0 == sock and "" == buffer) {
                throw EOFException();
            }
            while (sock != 0 and iter == buffer.end()) {
                std::vector<char> tmpbuf(8192, '\0');
                ssize_t status = 0;
                if (0 == (status = recv(sock, static_cast<void *>(&tmpbuf[0]), tmpbuf.size()-1, 0))) {
                    close(sock);
                    sock = 0;
                } else if ( -1 == status) {
                    throw NetworkError();
                }
                buffer += std::string(&tmpbuf[0]);
                iter = std::find(buffer.begin(), buffer.end(), '\n');
            }
            rv = std::string(buffer.begin(), iter);
            rv.erase(std::remove(rv.begin(), rv.end(), '\r'), rv.end());
            if (iter != buffer.end()) {
                iter++;
            }
            buffer = std::string(iter, buffer.end());
        }
        return rv;
    }

private:
    int sock;
    std::string buffer;
};

#endif // UNIX_HPP_INCLUDED
