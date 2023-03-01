#include <iostream>
#include "../lib/IClient.h"

int main() {
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
            std::cout << "Client response: " << response << std::endl;
        }
        else {
            std::cout << "Client request timeout" << std::endl;
        }
    }



    return 0;
}
