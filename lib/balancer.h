#pragma once

#include "IServer.h"
#include "ThreadSafeQueue.hpp"
#include <string>
#include <vector>
#include <thread>
#include <atomic>

namespace RpcLite {
    class Balancer {
    public:
        Balancer(int threadCount);
        ~Balancer();
        void enqueue(std::string&& data); // da to do najmenej vytazeneho bude balancovat do prislusnych queue
        std::string dequeue(int threadIndex);
        void start(IWorker& iWorker, IServer& IServer);
        void stop();

    private:
        int _threadCount;
        std::atomic<int> _currentIndex;
        std::vector<std::thread> _workerThreads;
        std::vector<ThreadSafeQueue<std::string>*> _threadSafeQueueVector;
        std::vector<std::atomic<int>*> _usageVector;
    };

}

