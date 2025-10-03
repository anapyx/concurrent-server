#pragma once

#include <string>
#include <atomic>
#include <memory>
#include <chrono>

class ThreadPool;
class ConnectionQueue;
class HttpHandler;

struct ServerStats {
    uint64_t totalConnections;
    uint64_t activeConnections;
    uint64_t totalRequests;
    uint64_t successfulRequests;
    uint64_t failedRequests;
    std::chrono::steady_clock::time_point startTime;
};

class HttpServer {
public:
    HttpServer(int port = 8080, const std::string& documentRoot = "./www");
    ~HttpServer();
    
    bool start();
    void stop();
    bool isRunning() const;
    
    ServerStats getStats() const;
    void printStats() const;

private:
    int port_;
    std::string documentRoot_;
    std::atomic<bool> running_;
    
    std::unique_ptr<ThreadPool> threadPool_;
    std::unique_ptr<ConnectionQueue> connectionQueue_;
    std::unique_ptr<HttpHandler> httpHandler_;
};