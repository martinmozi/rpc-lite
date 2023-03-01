#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <stack>

template <class T> class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(ThreadSafeQueue && other) = default;

    // enqueue - supports move, copies only if needed. e.g. q.enqueue(move(obj));
    void enqueue(T t) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(std::move(t));
        _cv.notify_one();
    }

    T dequeue() {
        std::unique_lock<std::mutex> lock(_mutex);

        _cv.wait(lock, [&]() { return !this->_queue.empty(); });
        T rVal = std::move(_queue.front());
        _queue.pop();
        return rVal;
    }

    bool dequeue(double timeout_sec, T& rVal) {
        bool isTimeout = false;
        auto maxTime = std::chrono::milliseconds(int(timeout_sec * 1000));

        std::unique_lock<std::mutex> lock(_mutex);

        if (_cv.wait_for(lock, maxTime, [&]() { return !this->_queue.empty(); })) {
            rVal = std::move(_queue.front());
            _queue.pop();
            return true;
        } else {
            return false;
        }
    }

    void clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue = std::queue<T>();
    }

    size_t size() const{
        std::lock_guard<std::mutex> lock(_mutex);
        return _queue.size();
    }

private:
    std::queue<T> _queue;
    mutable std::mutex _mutex;
    std::condition_variable _cv;
};
