#include "connection_queue.h"

ConnectionQueue::ConnectionQueue(size_t maxSize)
    : maxSize_(maxSize), shutdown_(false) {
}

bool ConnectionQueue::push(SOCKET socket, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    if (!condition_.wait_for(lock, timeout, [this] {
        return shutdown_ || queue_.size() < maxSize_;
    })) {
        return false;
    }
    
    if (shutdown_) {
        return false;
    }
    
    queue_.push(socket);
    condition_.notify_one();
    return true;
}

bool ConnectionQueue::pop(SOCKET& socket, std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    
    if (timeout == std::chrono::milliseconds::max()) {
        condition_.wait(lock, [this] {
            return shutdown_ || !queue_.empty();
        });
    } else {
        if (!condition_.wait_for(lock, timeout, [this] {
            return shutdown_ || !queue_.empty();
        })) {
            return false;
        }
    }
    
    if (shutdown_ && queue_.empty()) {
        return false;
    }
    
    socket = queue_.front();
    queue_.pop();
    condition_.notify_one();
    return true;
}

size_t ConnectionQueue::size() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
}

size_t ConnectionQueue::maxSize() const {
    return maxSize_;
}

bool ConnectionQueue::empty() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.empty();
}

void ConnectionQueue::shutdown() {
    std::unique_lock<std::mutex> lock(mutex_);
    shutdown_ = true;
    condition_.notify_all();
}