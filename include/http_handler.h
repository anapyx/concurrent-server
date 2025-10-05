#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h>
    typedef SOCKET SOCKET;
#else
    typedef int SOCKET;
#endif

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;
    bool keepAlive = false;
};

struct HttpResponse {
    int statusCode = 200;
    std::string statusText = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

struct KeepAliveConfig {
    int timeoutSeconds = 5;
    int maxRequests = 100;
};

class HttpHandler {
public:
    explicit HttpHandler(const std::string& documentRoot, const KeepAliveConfig& config = {});
    
    void handleConnection(SOCKET clientSocket);
    void handleConnectionWithKeepAlive(SOCKET clientSocket);
    
private:
    bool parseRequest(const std::string& requestData, HttpRequest& request);
    void handleGetRequest(const HttpRequest& request, HttpResponse& response);
    void sendResponse(SOCKET clientSocket, const HttpResponse& response, bool keepAlive = false);
    bool receiveRequestWithTimeout(SOCKET clientSocket, std::string& requestData);
    
    std::string getMimeType(const std::string& filename);
    std::string readFile(const std::string& filepath);
    bool fileExists(const std::string& filepath);
    
    std::string documentRoot_;
    KeepAliveConfig keepAliveConfig_;
};
