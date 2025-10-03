#pragma once

#include <string>
#include <unordered_map>

typedef int SOCKET;

// Estrutura da requisição e resposta HTTP (arquitetura inicial)

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
    
    // Interface principal para processamento de conexões
    void handleConnection(SOCKET clientSocket);
    
private:
    // Métodos de processamento (a serem implementados)
    bool parseRequest(const std::string& requestData, HttpRequest& request);
    void handleGetRequest(const HttpRequest& request, HttpResponse& response);
    void sendResponse(SOCKET clientSocket, const HttpResponse& response);
    
    // Utilitários para servir arquivos
    std::string getMimeType(const std::string& filename);
    std::string readFile(const std::string& filepath);
    bool fileExists(const std::string& filepath);
    
    std::string documentRoot_;
};
