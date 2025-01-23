#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include <condition_variable>
#include <deque>
#include <mutex>
#include <stdexcept>
#include <utility>

template<typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(size_t maxCapacity = 0) : capacity(maxCapacity), shutdownFlag(false) {}

    void put(const T &item) {
        std::unique_lock<std::mutex> lock(mutex);
        conditionVariable.wait(lock, [this]() {
            return !shutdownFlag && (!capacity || queue.size() < capacity);
        });
        if (shutdownFlag) {
            return;
        }
        queue.push_back(item);
        conditionVariable.notify_all();
    }

    void put(T &&item) {
        std::unique_lock<std::mutex> lock(mutex);
        conditionVariable.wait(lock, [this]() {
            return !shutdownFlag && (!capacity || queue.size() < capacity);
        });
        if (shutdownFlag) {
            return;
        }
        queue.push_back(std::move(item));
        conditionVariable.notify_all();
    }

    T take() {
        std::unique_lock<std::mutex> lock(mutex);
        conditionVariable.wait(lock, [this]() {
            return !queue.empty() || shutdownFlag;
        });
        if (queue.empty() && shutdownFlag) {
            throw std::runtime_error("BlockingQueue<T>::take: queue already shutdown");
        }
        T item = std::move(queue.front());
        queue.pop_front();
        conditionVariable.notify_all();
        return item;
    }

    void shutdown() {
        std::unique_lock<std::mutex> lock(mutex);
        shutdownFlag = true;
        conditionVariable.notify_all();
    }

    bool empty() const {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.empty();
    }

    size_t size() const {
        std::unique_lock<std::mutex> lock(mutex);
        return queue.size();
    }

private:
    std::deque<T> queue;
    mutable std::mutex mutex;
    std::condition_variable conditionVariable;
    size_t capacity;
    bool shutdownFlag;
};

#endif // BLOCKINGQUEUE_H