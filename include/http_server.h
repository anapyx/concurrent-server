#pragma once

#include <string>
#include <atomic>
#include <memory>
#include <chrono>
#include <thread>

#ifdef _WIN32
    #include <winsock2.h>
    typedef SOCKET SOCKET;
#else
    typedef int SOCKET;
#endif

class ThreadPool;
class ConnectionQueue;
class HttpHandler;

struct ServerStats {
    std::atomic<uint64_t> totalConnections{0};
    std::atomic<uint64_t> totalRequests{0};
    std::atomic<uint64_t> successfulRequests{0};
    std::atomic<uint64_t> failedRequests{0};
    std::atomic<uint64_t> droppedConnections{0};
    std::atomic<uint64_t> averageResponseTime{0};
};

class HttpServer {
public:
    HttpServer(int port = 8080, size_t numThreads = 4, size_t maxConnections = 100, 
               const std::string& documentRoot = "./www");
    ~HttpServer();
    
    bool start();
    void stop();
    bool isRunning() const;
    
    const ServerStats& getStats() const;
    void printStats() const;

private:
    void startWorkers();
    void acceptConnections();
    void workerLoop();
    
    int port_;
    size_t numThreads_;
    size_t maxConnections_;
    std::string documentRoot_;
    
    SOCKET serverSocket_;
    std::atomic<bool> running_;
    
    std::unique_ptr<ThreadPool> threadPool_;
    std::unique_ptr<ConnectionQueue> connectionQueue_;
    std::unique_ptr<HttpHandler> httpHandler_;
    
    std::thread acceptThread_;
    std::shared_ptr<ServerStats> stats_;
};