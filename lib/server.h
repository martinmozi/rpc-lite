#pragma once

#include "IServer.h"
#include "balancer.h"
#include <thread>

namespace RpcLite {
    // pre windows skusime wepoll
    class Server : public IServer {
    public:
        Server(int port, int threadCount);
        virtual ~Server();
        virtual void run(IWorker&& iWorker);
        virtual void stop();

        virtual bool waitForData(int threadIndex, int& clientSocket, std::string& data);
        virtual int send(int clientSocket, const std::string& data);

    private:
        int _port;
        int _threadCount;
        Balancer _balancer;
        std::vector<std::thread> _workers;
    };
}
