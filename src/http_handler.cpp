#include "http_handler.h"
#include "logger.h"
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #define closesocket close
#endif

HttpHandler::HttpHandler(const std::string& documentRoot, const KeepAliveConfig& config) 
    : documentRoot_(documentRoot), keepAliveConfig_(config) {
    
    if (!std::filesystem::exists(documentRoot_)) {
        std::filesystem::create_directories(documentRoot_);
        Logger::getInstance().info("Created document root: " + documentRoot_);
    }
}

void HttpHandler::handleConnection(SOCKET clientSocket) {
    handleConnectionWithKeepAlive(clientSocket);
}

void HttpHandler::handleConnectionWithKeepAlive(SOCKET clientSocket) {
    int requestCount = 0;
    
    while (requestCount < keepAliveConfig_.maxRequests) {
        std::string requestData;
        
        if (!receiveRequestWithTimeout(clientSocket, requestData)) {
            break; // Timeout ou erro
        }
        
        HttpRequest request;
        HttpResponse response;
        
        if (parseRequest(requestData, request)) {
            Logger::getInstance().info("HTTP " + request.method + " " + request.path + 
                                     " - Processing (req #" + std::to_string(requestCount + 1) + ")");
            
            if (request.method == "GET") {
                handleGetRequest(request, response);
            } else {
                response.statusCode = 405;
                response.statusText = "Method Not Allowed";
                response.body = "<html><body><h1>405 Method Not Allowed</h1></body></html>";
            }
        } else {
            response.statusCode = 400;
            response.statusText = "Bad Request";
            response.body = "<html><body><h1>400 Bad Request</h1></body></html>";
        }
        
        bool shouldKeepAlive = request.keepAlive && (requestCount + 1 < keepAliveConfig_.maxRequests);
        sendResponse(clientSocket, response, shouldKeepAlive);
        
        Logger::getInstance().info("HTTP " + request.method + " " + request.path + 
                                 " - Response " + std::to_string(response.statusCode) +
                                 (shouldKeepAlive ? " (keep-alive)" : " (close)"));
        
        requestCount++;
        
        if (!shouldKeepAlive) {
            break;
        }
    }
    
    closesocket(clientSocket);
}

bool HttpHandler::parseRequest(const std::string& requestData, HttpRequest& request) {
    std::istringstream stream(requestData);
    std::string line;
    
    // Parse primeira linha
    if (!std::getline(stream, line)) {
        return false;
    }
    
    std::istringstream firstLine(line);
    if (!(firstLine >> request.method >> request.path >> request.version)) {
        return false;
    }
    
    // Parse headers
    while (std::getline(stream, line) && !line.empty() && line != "\r") {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string headerName = line.substr(0, colonPos);
            std::string headerValue = line.substr(colonPos + 1);
            
            // Remove espaços
            while (!headerValue.empty() && headerValue[0] == ' ') {
                headerValue = headerValue.substr(1);
            }
            while (!headerValue.empty() && headerValue.back() == '\r') {
                headerValue.pop_back();
            }
            
            // Converter para lowercase para comparação
            std::transform(headerName.begin(), headerName.end(), headerName.begin(), ::tolower);
            request.headers[headerName] = headerValue;
        }
    }
    
    // Verificar keep-alive
    auto connectionIt = request.headers.find("connection");
    if (connectionIt != request.headers.end()) {
        std::string connectionValue = connectionIt->second;
        std::transform(connectionValue.begin(), connectionValue.end(), connectionValue.begin(), ::tolower);
        request.keepAlive = (connectionValue == "keep-alive");
    }
    
    return true;
}

void HttpHandler::handleGetRequest(const HttpRequest& request, HttpResponse& response) {
    std::string filePath = documentRoot_ + request.path;
    
    if (request.path.back() == '/') {
        filePath += "index.html";
    }
    
    if (fileExists(filePath)) {
        response.body = readFile(filePath);
        response.headers["Content-Type"] = getMimeType(filePath);
        response.headers["Content-Length"] = std::to_string(response.body.length());
    } else {
        response.statusCode = 404;
        response.statusText = "Not Found";
        response.body = "<html><body><h1>404 Not Found</h1></body></html>";
        response.headers["Content-Type"] = "text/html";
    }
}

void HttpHandler::sendResponse(SOCKET clientSocket, const HttpResponse& response, bool keepAlive) {
    std::ostringstream responseStream;
    
    responseStream << "HTTP/1.1 " << response.statusCode << " " << response.statusText << "\r\n";
    
    // Adicionar headers da response
    for (const auto& header : response.headers) {
        responseStream << header.first << ": " << header.second << "\r\n";
    }
    
    // Adicionar Content-Length
    responseStream << "Content-Length: " << response.body.length() << "\r\n";
    
    // Adicionar Connection header
    if (keepAlive) {
        responseStream << "Connection: keep-alive\r\n";
        responseStream << "Keep-Alive: timeout=" << keepAliveConfig_.timeoutSeconds 
                      << ", max=" << keepAliveConfig_.maxRequests << "\r\n";
    } else {
        responseStream << "Connection: close\r\n";
    }
    
    responseStream << "\r\n" << response.body;
    
    std::string responseStr = responseStream.str();
    send(clientSocket, responseStr.c_str(), responseStr.length(), 0);
}

std::string HttpHandler::getMimeType(const std::string& filename) {
    auto pos = filename.find_last_of('.');
    if (pos == std::string::npos) {
        return "application/octet-stream";
    }
    
    std::string extension = filename.substr(pos + 1);
    
    if (extension == "html" || extension == "htm") return "text/html";
    if (extension == "txt") return "text/plain";
    
    return "application/octet-stream";
}

std::string HttpHandler::readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

bool HttpHandler::fileExists(const std::string& filepath) {
    return std::filesystem::exists(filepath) && std::filesystem::is_regular_file(filepath);
}

bool HttpHandler::receiveRequestWithTimeout(SOCKET clientSocket, std::string& requestData) {
#ifdef _WIN32
    // Configurar timeout no Windows
    DWORD timeout = keepAliveConfig_.timeoutSeconds * 1000;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
    // Configurar timeout no Linux
    struct timeval timeout;
    timeout.tv_sec = keepAliveConfig_.timeoutSeconds;
    timeout.tv_usec = 0;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif
    
    char buffer[4096];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesReceived <= 0) {
        return false; // Timeout ou erro
    }
    
    buffer[bytesReceived] = '\0';
    requestData = std::string(buffer);
    return true;
}
