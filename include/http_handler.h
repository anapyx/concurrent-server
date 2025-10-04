#pragma once

#include <string>
#include <unordered_map>
#include <memory>

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
};

struct HttpResponse {
    int statusCode = 200;
    std::string statusText = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;
};

class HttpHandler {
public:
    explicit HttpHandler(const std::string& documentRoot);
    
    void handleConnection(SOCKET clientSocket);
    
private:
    bool parseRequest(const std::string& requestData, HttpRequest& request);
    void handleGetRequest(const HttpRequest& request, HttpResponse& response);
    void sendResponse(SOCKET clientSocket, const HttpResponse& response);
    
    std::string getMimeType(const std::string& filename);
    std::string readFile(const std::string& filepath);
    bool fileExists(const std::string& filepath);
    
    std::string documentRoot_;
};
