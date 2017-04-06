#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <boost/optional.hpp>

namespace beton {

    /// A simple thread-safe queue.
    template <typename T>
    class Queue {
        std::queue<T> queue;
        std::mutex mutex;
        std::condition_variable condition;

    public:
        Queue() = default;
        Queue(const Queue&) = delete; // disable copying
        Queue& operator=(const Queue&) = delete; // disable assignment

        T pop() {
            std::unique_lock<std::mutex> lock(mutex);

            while (queue.empty()) {
                condition.wait(lock);
            }

            auto value = queue.front();
            queue.pop();
            
            return value;
        }

        boost::optional<T> tryPop(std::chrono::milliseconds waitFor) {
            std::unique_lock<std::mutex> lock(mutex);

            while (queue.empty()) {
                if (condition.wait_for(lock, waitFor) == std::cv_status::timeout) {
                    return boost::none;
                }
            }

            auto value = queue.front();
            queue.pop();
            
            return value;
        }

        void push(const T& item) {
            std::unique_lock<std::mutex> lock(mutex);

            queue.push(item);

            lock.unlock();
            condition.notify_one();
        }

        bool isEmpty() const {
            std::unique_lock<std::mutex> lock;
            return queue.empty();
        }
    };

    template<typename T> using QueueRef = std::shared_ptr<Queue<T>>;
}
