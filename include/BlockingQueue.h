#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>
class BlockingQueue {
public:
  explicit BlockingQueue() {};
  explicit BlockingQueue(size_t maxCapacity);

  void put(const T& item);
  void put(T&& item);
  T take();

  void shutdown();
  bool empty() const;
  size_t size() const;

private:
  std::deque<T> queue;
  mutable std::mutex mutex;
  std::condition_variable conditionVariable;
  std::optional<size_t> capacity;
  bool shutdownFlag = false;
};

#endif //BLOCKINGQUEUE_H
