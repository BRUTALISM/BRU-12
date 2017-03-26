#ifndef Queue_hpp
#define Queue_hpp

#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

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

        T pop();
        void push(const T& item);
        bool isEmpty() const;
    };

}

#endif /* Queue_hpp */
