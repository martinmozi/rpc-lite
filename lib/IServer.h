#pragma once

#include <string>
#include <memory>

namespace RpcLite {
    // must be defined by user
    class IServer;
    class IWorker {
    public:
        virtual ~IWorker() = default;
        virtual void run(int threadIndex, IServer& iServer) = 0;
    };

    class IServer {
    public:
        virtual ~IServer() = default;
        virtual void run(IWorker&& iWorker) = 0;
        virtual void stop() = 0;

        virtual bool waitForData(int threadIndex, int& clientSocket, std::string& data) = 0;
        virtual void send(int clientSocket, const std::string& data) = 0;
    };

    std::unique_ptr<IServer> createServer(int port, int threadCount);
}
