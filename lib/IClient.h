#pragma once

#include <string>
#include <memory>

namespace RpcLite {
    class IClient {
    public:
        virtual ~IClient() = default;
        virtual bool send(const std::string& methodData, std::string& outData) = 0;
        virtual bool connect(int port, int timeoutMs) = 0;
        virtual bool connect() = 0;
        virtual void disconnect() = 0;
    };

    std::unique_ptr<IClient> createClient(int port, int timeoutMs);
}
