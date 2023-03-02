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

namespace {
    int sendData(SOCKET clientSocket, const std::string& data) {
#ifdef WIN32
        return ::send(clientSocket, data.c_str(), (int)data.size(), 0);
#else
        return ::send(clientSocket, data.c_str(), data.size(), 0);
#endif
    }
}

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


bool RpcLite::Client::receive(std::string& data) {
    char buffer[8152];

    struct timeval tv;
    tv.tv_sec = _timeoutMs / 1000;
    tv.tv_usec = (_timeoutMs % 1000) * 1000;

    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(_sock, &fdset);

    if (::select((int)_sock + 1, &fdset, NULL, NULL, &tv) == 1) {
        int n = ::recv(_sock, buffer, sizeof(buffer), 0);
        if (n > 0) {
            data = std::string(buffer, n);
            return true;
        }
        else {
            printf("Received -1\n");
            return false;
        }
    }
    
    printf("Reading timeout timeout\n");
    return false;
}

bool RpcLite::Client::send(const std::string & methodData, std::string & outData) {
    if (_sock <= 0) {
        return false;
    }

    if (sendData(_sock, methodData) > 0) {
        return receive(outData);
    }

    printf("Sending error\n");
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

    struct timeval tv;
    tv.tv_sec = _timeoutMs / 1000;
    tv.tv_usec = (_timeoutMs % 1000) * 1000;
    _sock = socket(AF_INET, SOCK_STREAM, 0);
    Os::set_blocking_mode(_sock, false);
    ::connect(_sock, (struct sockaddr*)&address, sizeof(address));

    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(_sock, &fdset);

    if (::select((int)_sock + 1, NULL, &fdset, NULL, &tv) == 1)
    {
        int so_error;
        socklen_t len = sizeof(so_error);
        ::getsockopt(_sock, SOL_SOCKET, SO_ERROR, (char*)&so_error, &len);
        if (so_error == 0) {
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
