#pragma once

#include "IClient.h"
#include "os.h"

namespace RpcLite {
    class Client : public IClient {
    public:
        Client(int port, int timeoutMs);
        virtual ~Client();
        virtual bool send(const std::string& methodData, std::string& outData);
        virtual bool connect(int port, int timeoutMs);
        virtual bool connect();
        virtual void disconnect();

    private:
        bool receive(std::string& data);

    private:
        int _port;
        int _timeoutMs;
        SOCKET _sock;
    };
}
