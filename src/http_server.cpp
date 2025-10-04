#include "http_server.h"
#include "thread_pool.h"
#include "connection_queue.h"
#include "http_handler.h"
#include "logger.h"
#include <stdexcept>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define closesocket close
    #define INVALID_SOCKET -1
    #define SOCKET int
#endif

HttpServer::HttpServer(int port, size_t numThreads, size_t maxConnections, const std::string& documentRoot)
    : port_(port), 
      documentRoot_(documentRoot),
      running_(false),
      stats_(std::make_shared<ServerStats>()),
      threadPool_(std::make_unique<ThreadPool>(numThreads)),
      connectionQueue_(std::make_unique<ConnectionQueue>(maxConnections)),
      httpHandler_(std::make_unique<HttpHandler>(documentRoot_)),
      serverSocket_(INVALID_SOCKET) {
    
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        throw std::runtime_error("Failed to initialize Winsock");
    }
#endif
    
    Logger::getInstance().info("HTTP Server initialized on port " + std::to_string(port_) + 
                              " with " + std::to_string(numThreads) + " threads");
}

HttpServer::~HttpServer() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

bool HttpServer::start() {
    if (running_.load()) {
        Logger::getInstance().warning("Server is already running");
        return false;
    }
    
    serverSocket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket_ == INVALID_SOCKET) {
        throw std::runtime_error("Failed to create socket");
    }
    
    int reuse = 1;
    setsockopt(serverSocket_, SOL_SOCKET, SO_REUSEADDR, 
              reinterpret_cast<const char*>(&reuse), sizeof(reuse));
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port_);
    
    if (bind(serverSocket_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        closesocket(serverSocket_);
        throw std::runtime_error("Failed to bind socket to port " + std::to_string(port_));
    }
    
    if (listen(serverSocket_, SOMAXCONN) < 0) {
        closesocket(serverSocket_);
        throw std::runtime_error("Failed to listen on socket");
    }
    
    running_.store(true);
    Logger::getInstance().info("Server started and listening on port " + std::to_string(port_));
    
    startWorkers();
    acceptConnections();
    return true;
}

void HttpServer::stop() {
    if (!running_.load()) {
        return;
    }
    
    Logger::getInstance().info("Stopping server...");
    running_.store(false);
    
    if (serverSocket_ != INVALID_SOCKET) {
        closesocket(serverSocket_);
        serverSocket_ = INVALID_SOCKET;
    }
    
    connectionQueue_->shutdown();
    threadPool_.reset();
    
    Logger::getInstance().info("Server stopped");
}

bool HttpServer::isRunning() const {
    return running_.load();
}

const ServerStats& HttpServer::getStats() const {
    return *stats_;
}

void HttpServer::startWorkers() {
    size_t numThreads = threadPool_->size();
    
    for (size_t i = 0; i < numThreads; ++i) {
        threadPool_->enqueue([this]() {
            workerLoop();
        });
    }
    
    Logger::getInstance().info("Started " + std::to_string(numThreads) + " worker threads");
}

void HttpServer::acceptConnections() {
    while (running_.load()) {
        sockaddr_in clientAddr{};
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        SOCKET clientSocket = accept(serverSocket_, 
                                   reinterpret_cast<sockaddr*>(&clientAddr), 
                                   &clientAddrLen);
        
        if (clientSocket == INVALID_SOCKET) {
            if (running_.load()) {
                Logger::getInstance().error("Failed to accept connection");
            }
            continue;
        }
        
        stats_->totalConnections.fetch_add(1);
        
        std::string clientIP = inet_ntoa(clientAddr.sin_addr);
        Logger::getInstance().debug("Accepted connection from " + clientIP);
        
        if (!connectionQueue_->push(clientSocket)) {
            Logger::getInstance().warning("Connection queue full, dropping connection from " + clientIP);
            closesocket(clientSocket);
            stats_->droppedConnections.fetch_add(1);
        }
    }
}

void HttpServer::workerLoop() {
    while (running_.load() || !connectionQueue_->empty()) {
        SOCKET clientSocket;
        
        if (connectionQueue_->pop(clientSocket, std::chrono::milliseconds(100))) {
            auto startTime = std::chrono::steady_clock::now();
            
            try {
                httpHandler_->handleConnection(clientSocket);
                stats_->successfulRequests.fetch_add(1);
            } catch (const std::exception& e) {
                Logger::getInstance().error("Error handling connection: " + std::string(e.what()));
                stats_->failedRequests.fetch_add(1);
                closesocket(clientSocket);
            }
            
            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
            
            uint64_t oldAvg = stats_->averageResponseTime.load();
            uint64_t newTime = duration.count();
            uint64_t totalReqs = stats_->successfulRequests.load() + stats_->failedRequests.load();
            
            if (totalReqs > 0) {
                uint64_t newAvg = (oldAvg * (totalReqs - 1) + newTime) / totalReqs;
                stats_->averageResponseTime.store(newAvg);
            }
        }
    }
}
