#include "balancer.h"
#include <limits>

RpcLite::Balancer::Balancer(int threadCount)
: _threadCount(threadCount)
, _currentIndex(0) {
}

RpcLite::Balancer::~Balancer() {
}

void RpcLite::Balancer::start(RpcLite::IWorker& iWorker, RpcLite::IServer& iServer) {
	stop();
	for (int i = 0; i < _threadCount; i++) {
		// objects are not movable, pointer has to be used, also unique poiter are possible, but for performance reasons
		_threadSafeQueueVector.push_back(new ThreadSafeQueue<std::string>());
		_usageVector.emplace_back(new std::atomic<int>(0));
	}

	for (int i = 0; i < _threadCount; i++) {
		_workerThreads.push_back(std::move(std::thread([this, &iWorker , &iServer, i]() { iWorker.run(i, iServer); })));
	}
}

void RpcLite::Balancer::stop() {
	for (auto& queue : _threadSafeQueueVector) {
		queue->enqueue(std::string(""));
	}

	for (std::thread& t : _workerThreads) {
		t.join();
	}

	_workerThreads.clear();

	for (size_t i = 0; i < _threadSafeQueueVector.size(); i++) {
		delete _threadSafeQueueVector.at(i);
		delete _usageVector.at(i);
	}

	_threadSafeQueueVector.clear();
	_usageVector.clear();
}

void RpcLite::Balancer::enqueue(std::string&& data) {
	// Todo: improve this - not only send to the queue with minimum data but consider also the total amount of messages 
	int minCount = std::numeric_limits<int>::max();
	int currentIndex = 0;
	for (int i = 0; i < _threadCount; i++) {
		int h = *_usageVector.at(i);
		if (h < minCount) {
			minCount = h;
			currentIndex = i;
		}
	}

	_usageVector.at(currentIndex)->fetch_add(1, std::memory_order_relaxed);
	_threadSafeQueueVector.at(currentIndex)->enqueue(std::move(data));
}

std::string RpcLite::Balancer::dequeue(int threadIndex) {
	_usageVector.at(threadIndex)->fetch_add(-1, std::memory_order_relaxed);
	return _threadSafeQueueVector.at(threadIndex)->dequeue();
}
