#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template <typename T>
class ThreadSafeQueue {
    std::queue<T> queue_;
    mutable std::mutex mtx_;
    std::condition_variable cond_var_;
public:
    ThreadSafeQueue() {}

    // Add an item to the queue
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(item);
        cond_var_.notify_one();  // Notify one waiting thread
    }

    // Add an item to the queue (move version)
    void push(T&& item) {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.push(std::move(item));
        cond_var_.notify_one();  // Notify one waiting thread
    }

    // Get an item from the queue, waits if empty
    T waitPop() {
        std::unique_lock<std::mutex> lock(mtx_);

        cond_var_.wait(lock, [this] { return !queue_.empty(); });
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }

    // Try to get an item without waiting; returns std::optional<T>
    std::optional<T> tryPop() {
        std::lock_guard<std::mutex> lock(mtx_);
        if (queue_.empty()) return std::nullopt;
        T item = std::move(queue_.front());
        queue_.pop();
        return item;
    }


    // Check if the queue is empty
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.empty();
    }
};
