#pragma once

#include <condition_variable>
#include <queue>
#include <mutex>
#include <optional>

namespace rbmq::utils {

template <typename T>
class ConcurentQueue {
private:
    // Underlying non-thread safe queue
    std::queue<T> _queue;
  
    // Mutex for thread synchronization
    std::mutex _mutex;
  
    // Condition variable for notifying
    std::condition_variable _cond;
  
public:
    void push(T item)
    {
        {
            // Lock: prevents data race and allows notifying
            std::scoped_lock<std::mutex> lock(_mutex);
            _queue.push(item);
        }
        // Notify that a new data is available to consumers
        _cond.notify_one();
    }

    std::optional<T> try_pop()
    {
        // Lock: prevents data race and allows correct notifying
        std::scoped_lock<std::mutex> lock(_mutex);

        // If queue is empty return empty
        if (_queue.empty())
            return std::nullopt;

        // Retrieve item from the queue and return it
        auto item = std::move(_queue.front());
        _queue.pop();

        return item;
    }
  
    T pop()
    {
        // Lock: prevents data race and allows correct notifying
        std::scoped_lock<std::mutex> lock(_mutex);
        // Wait until queue is not empty
        _cond.wait(lock, [this]() { return !_queue.empty(); });
        // Retrieve item from the queue and return it
        auto item = std::move(_queue.front());
        _queue.pop();

        return item;
    }
};

}
