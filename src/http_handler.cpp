#include "http_handler.h"
#include "logger.h"
#include <sstream>
#include <fstream>
#include <filesystem>

#ifdef _WIN32
    #include <winsock2.h>
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #define closesocket close
#endif

HttpHandler::HttpHandler(const std::string& documentRoot) 
    : documentRoot_(documentRoot) {
    
    if (!std::filesystem::exists(documentRoot_)) {
        std::filesystem::create_directories(documentRoot_);
        Logger::getInstance().info("Created document root: " + documentRoot_);
    }
}

void HttpHandler::handleConnection(SOCKET clientSocket) {
    char buffer[4096];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesReceived <= 0) {
        Logger::getInstance().error("Failed to receive data from client");
        closesocket(clientSocket);
        return;
    }
    
    buffer[bytesReceived] = '\0';
    std::string requestData(buffer);
    
    HttpRequest request;
    HttpResponse response;
    
    if (parseRequest(requestData, request)) {
        Logger::getInstance().info("Processing " + request.method + " " + request.path);
        
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
    
    sendResponse(clientSocket, response);
    closesocket(clientSocket);
}

bool HttpHandler::parseRequest(const std::string& requestData, HttpRequest& request) {
    std::istringstream stream(requestData);
    std::string line;
    
    if (!std::getline(stream, line)) {
        return false;
    }
    
    std::istringstream firstLine(line);
    if (!(firstLine >> request.method >> request.path >> request.version)) {
        return false;
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

void HttpHandler::sendResponse(SOCKET clientSocket, const HttpResponse& response) {
    std::ostringstream responseStream;
    
    responseStream << "HTTP/1.0 " << response.statusCode << " " << response.statusText << "\r\n";
    
    for (const auto& header : response.headers) {
        responseStream << header.first << ": " << header.second << "\r\n";
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
