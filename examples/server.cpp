#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include "../lib/IServer.h"

class Worker : public RpcLite::IWorker {
public:
    Worker() {}
    virtual void run(int threadIndex, RpcLite::IServer& iServer) {
        std::string fileName("xx_" + std::to_string(threadIndex) + ".log");
        std::ofstream fs(fileName);
        int clientSocket;
        std::string data;
        while (iServer.waitForData(threadIndex, clientSocket, data)) {
            std::string outData = data + std::to_string(threadIndex) + "\n";
            iServer.send(clientSocket, outData);

            // do some time consuming operation
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            fs << outData << std::flush;
        }
    }
};

int main() {
    const int port = 10002;
    const int threadCount = 4;
    auto iServer = RpcLite::createServer(port, threadCount);
    iServer->run(std::move(Worker()));
    return 0;
}