#include "client.h"
#ifdef WIN32
    #include <ws2tcpip.h>
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <unistd.h>
#endif

std::unique_ptr<RpcLite::IClient> RpcLite::createClient(int port, int timeoutMs) {
    return std::make_unique<RpcLite::Client>(port, timeoutMs);
}

RpcLite::Client::Client(int port, int timeoutMs)
: _port(port) 
, _timeoutMs(timeoutMs)
, _sock(0) {
}

RpcLite::Client::~Client() {
    disconnect();
}

bool RpcLite::Client::send(const std::string & methodData, std::string & outData) {
    char buffer[8152];
    if (_sock <= 0) {
        return false;
    }

    if (::send(_sock, methodData.c_str(), methodData.size(), 0) > 0) {
        int n = ::recv(_sock, buffer, sizeof(buffer), 0);
        if (n > 0) {
            outData = std::string(buffer, n);
            return true;
        }
        else {
            return false;
        }
    }

    // Todo: log some error 
    return false;
}

bool RpcLite::Client::connect(int port, int timeoutMs) {
    _port = port;
    _timeoutMs = timeoutMs;
    return connect();
}

bool RpcLite::Client::connect() {
    disconnect();

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &address.sin_addr.s_addr); /* assign the address */
    address.sin_port = htons(_port);            /* translate int2port num */

    _sock = socket(AF_INET, SOCK_STREAM, 0);
    Os::set_blocking_mode(_sock, false);
    ::connect(_sock, (struct sockaddr*)&address, sizeof(address));

    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(_sock, &fdset);

    struct timeval tv;
    tv.tv_sec = _timeoutMs / 1000;
    tv.tv_usec = (_timeoutMs % 1000) * 1000;
    setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));

    if (::select(_sock + 1, NULL, &fdset, NULL, &tv) == 1)
    {
        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(_sock, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len);
        if (so_error == 0) {
            Os::set_blocking_mode(_sock, true);
            return true;
        }

        printf("%d is not open\n", _port);
    }
    else {
        printf("Connection timeout");
    }

    return false;
}

void RpcLite::Client::disconnect() {
    if (_sock > 0) {
        Os::close_socket(_sock);
        _sock = 0;
    }
}
