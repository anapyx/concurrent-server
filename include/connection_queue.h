#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <memory>
#include <atomic>

#ifdef _WIN32
    #include <winsock2.h>
    typedef SOCKET SOCKET;
#else
    typedef int SOCKET;
#endif

class ConnectionQueue {
public:
    explicit ConnectionQueue(size_t maxSize);
    
    bool push(SOCKET socket, std::chrono::milliseconds timeout = std::chrono::milliseconds(1000));
    bool pop(SOCKET& socket, std::chrono::milliseconds timeout = std::chrono::milliseconds::max());
    
    size_t size() const;
    size_t maxSize() const;
    bool empty() const;
    void shutdown();

private:
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::queue<SOCKET> queue_;
    size_t maxSize_;
    std::atomic<bool> shutdown_;
};
