# include <../include/BlockingQueue.h>

/*
  std::deque<T> queue;
  mutable std::mutex mutex;
  std::condition_variable conditionVariable;
  std::optional<size_t> capacity;
  bool shutdownFlag = false;
 */
template<typename T>
BlockingQueue<T>::BlockingQueue(size_t maxCapacity) : capacity(maxCapacity) {}

template<typename T>
void BlockingQueue<T>::put(const T &item) {
  std::unique_lock<std::mutex> lock(mutex);
  conditionVariable.wait(
    // make thread to wait until runnable condition returns true.
    lock, [this]() {
      return !shutdownFlag && (!capacity || queue.size() < *capacity);
    }
  );
  if (shutdownFlag) {
    return;
  }
  queue.push_back(item);
  conditionVariable.notify_all();
}

template<typename T>
void BlockingQueue<T>::put(T &&item) {
  std::unique_lock<std::mutex> lock(mutex);
  conditionVariable.wait(
    lock, [this]() {
      return !shutdownFlag && (!capacity || queue.size() < *capacity);
    }
  );
  if (shutdownFlag) {
    return;
  }
  queue.push_back(std::move(item));
  conditionVariable.notify_all();
}

template<typename T>
T BlockingQueue<T>::take() {
  std::unique_lock<std::mutex> lock(mutex);
  conditionVariable.wait(
    lock, [this]() { return !queue.empty() || shutdownFlag; }
  );
  if (queue.empty() && shutdownFlag) {
    throw std::runtime_error("BlockingQueue<T>::take: queue already shutdown");
  }
  T item = std::move(queue.front());
  queue.pop_front();
  conditionVariable.notify_all();
  return item;
}

template<typename T>
void BlockingQueue<T>::shutdown() {
  std::unique_lock<std::mutex> lock(mutex);
  shutdownFlag = true;
  conditionVariable.notify_all();
}

template<typename T>
bool BlockingQueue<T>::empty() const {
  std::unique_lock<std::mutex> lock(mutex);
  return queue.empty();
}

template<typename T>
size_t BlockingQueue<T>::size() const {
  std::unique_lock<std::mutex> lock(mutex);
  return queue.size();
}
