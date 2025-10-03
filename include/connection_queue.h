#pragma once

#include <chrono>
#include <cstddef>

typedef int SOCKET;
 
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
    // Implementação será desenvolvida nas próximas partes
    size_t maxSize_;
    bool shutdown_;
};
