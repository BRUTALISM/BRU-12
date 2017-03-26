#include "Queue.hpp"

using namespace beton;

template <typename T>
T Queue<T>::pop() {
    std::unique_lock<std::mutex> lock(mutex);

    while (queue.empty()) {
        condition.wait(lock);
    }

    auto val = queue.front();
    queue.pop();

    return val;
}

template <typename T>
void Queue<T>::push(const T& item) {
    std::unique_lock<std::mutex> lock(mutex);

    queue.push(item);

    lock.unlock();
    condition.notify_one();
}

template <typename T>
bool Queue<T>::isEmpty() const {
    std::unique_lock<std::mutex> lock;
    return queue.empty();
}
