#if _WIN32 || __WIN32__ || __WINDOWS__ || _win32
#ifndef WIN32_HPP_INCLUDED
#define WIN32_HPP_INCLUDED
#define WINDOWS

#undef UNICODE

#include <ws2tcpip.h>
#include <array>
#include <algorithm>
#include <winsock2.h>
#include <windows.h>

class NetworkSocket {
public:
    NetworkSocket()
        : sock { INVALID_SOCKET }
        , buffer { "" }
    {
    }

    void connect(std::string host, unsigned short int port)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in server;
        //sockaddr_in *sockaddrIpv4 = 0;
        addrinfo *result { nullptr }, *ptr { nullptr };
        addrinfo hints;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        if (INVALID_SOCKET == sock) {
            std::cerr << "error : could not create a socket..." << std::endl;
            throw NetworkError();
        }
        if (0 != GetAddrInfo(host.c_str(), NULL, &hints, &result)) {
            std::cerr << "error : DNS failure (couldn't look up server).." << std::endl;
            throw NetworkError();
        }
        for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
            if (ptr->ai_family == AF_INET) {
                CopyMemory(&server, ptr->ai_addr, sizeof(server));
                break;
            }
        }
        if (nullptr == ptr) {
            std::cerr << "error : DNS failure ( server not found).." << std::endl;
            throw NetworkError();
        }
        freeaddrinfo(result);
        server.sin_family = AF_INET;
        server.sin_port = htons(port);
        if (SOCKET_ERROR == ::connect(sock, (sockaddr *)&server, sizeof(server))) {
            throw ConnectionRefused();
        }
    }

    bool isConnected() const { return (INVALID_SOCKET != sock); }

    void disconnect()
    {
        if (INVALID_SOCKET != sock) {
            closesocket(sock);
        }
        sock = INVALID_SOCKET;
    }

    virtual ~NetworkSocket() { disconnect(); }

    void write(const std::string &line)
    {
        if (SOCKET_ERROR == send(sock, line.c_str(), static_cast<int>(line.size()), 0)) {
            std::cerr << "error : couldn't send a packet to the server..." << std::endl;
            bomb(0);
        }
    }

    void write(const char *buf)
    {
        std::string line(buf);
        write(line);
    }

    std::string readLine()
    {
        auto sit = std::find(buffer.begin(), buffer.end(), '\n');
        std::string rv { "" };
        size_t bytesRead { 0 };
        fd_set fds;
        timeval tv;
        int selCall { 0 };
        tv.tv_sec = 0;
        tv.tv_usec = 500000;
        FD_ZERO(&fds);
        FD_SET(sock, &fds);
        if (!isConnected() && buffer != "") {
            rv = buffer;
            rv.erase(std::remove(rv.begin(), rv.end(), '\n'), rv.end());
            rv.erase(std::remove(rv.begin(), rv.end(), '\r'), rv.end());
            buffer = "";
            return rv;
        }
        if (!isConnected() && buffer == "") {
            throw EOFException();
        }
        while (isConnected() && sit == buffer.end()) {
            selCall = select(0, &fds, NULL, &fds, &tv);
            if (0 == selCall) {
                continue;
            }
            if (SOCKET_ERROR == selCall) {
                disconnect();
                throw NetworkError();
            }
            std::fill(temporaryPool.begin(), temporaryPool.end(), 0);
            bytesRead = recv(sock, &temporaryPool[0], static_cast<int>(temporaryPool.size()), 0);
            //if (SOCKET_ERROR == bytesRead || 0 == bytesRead) {
            if (0 == bytesRead) {
                disconnect();
                break;
            }
            buffer += std::string(&temporaryPool[0], &temporaryPool[0]+bytesRead);
            sit = std::find(buffer.begin(), buffer.end(), '\n');
        }
        sit = std::find(buffer.begin(), buffer.end(), '\n');
        rv = std::string(buffer.begin(), sit);
        if (sit != buffer.end()) {
            sit++;
        }
        buffer = std::string(sit, buffer.end());
        return rv;
    }

private:
    SOCKET sock;
    std::string buffer;
    std::array<char, 8192> temporaryPool;
};

#endif // WIN32_HPP_INCLUDED
#endif
