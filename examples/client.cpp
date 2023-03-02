#include <iostream>
#include "../lib/IClient.h"
#ifdef WIN32
#include <winsock2.h>
#endif

int main() {
#ifdef WIN32
    WSADATA wsaData;
    ::WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    const int port = 10002;
    const int timeout = 2000; //ms
    auto iClient = RpcLite::createClient(port, timeout);
    if (!iClient->connect()) {
        std::cout << "Unable to connect to server on port: " << port << std::endl;
        return -1;
    }

    std::string response;
    std::string request("some request ");
    for (int i = 0; i < 10; i++) {
        std::string r = request + std::to_string(i);
        if (iClient->send(request, response)) {
            std::cout << "Client response: " << response;
        }
        else {
            std::cout << "Client request timeout" << std::endl;
        }
    }

#ifdef WIN32
    ::WSACleanup();
#endif

    return 0;
}
