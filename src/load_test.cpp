#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>

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
    #define SOCKET int
#endif

std::atomic<int> successfulRequests(0);
std::atomic<int> failedRequests(0);
std::atomic<long long> totalResponseTime(0);

void clientWorker(const std::string& host, int port, int numRequests, int clientId) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> delay(100, 500); // Random delay between requests
    
    std::vector<std::string> paths = {"/", "/index.html", "/test.txt"};
    std::uniform_int_distribution<> pathDist(0, paths.size() - 1);
    
    for (int i = 0; i < numRequests; ++i) {
        auto start = std::chrono::steady_clock::now();
        
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket < 0) {
            failedRequests++;
            continue;
        }
        
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);
        
        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
            closesocket(clientSocket);
            failedRequests++;
            continue;
        }
        
        std::string path = paths[pathDist(gen)];
        std::string request = "GET " + path + " HTTP/1.0\r\n";
        request += "Host: " + host + "\r\n";
        request += "User-Agent: LoadTester-Client-" + std::to_string(clientId) + "\r\n";
        request += "Connection: close\r\n\r\n";
        
        if (send(clientSocket, request.c_str(), request.length(), 0) < 0) {
            closesocket(clientSocket);
            failedRequests++;
            continue;
        }
        
        char buffer[4096];
        int totalReceived = 0;
        while (recv(clientSocket, buffer, sizeof(buffer), 0) > 0) {
            totalReceived++;
        }
        
        closesocket(clientSocket);
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        successfulRequests++;
        totalResponseTime += duration.count();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(delay(gen)));
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "Usage: " << argv[0] << " <host> <port> <num_clients> <requests_per_client>\n";
        std::cout << "Example: " << argv[0] << " 127.0.0.1 8080 10 5\n";
        return 1;
    }
    
    std::string host = argv[1];
    int port = std::stoi(argv[2]);
    int numClients = std::stoi(argv[3]);
    int requestsPerClient = std::stoi(argv[4]);
    
    std::cout << "=== Concurrent HTTP Load Test ===\n";
    std::cout << "Target: " << host << ":" << port << "\n";
    std::cout << "Clients: " << numClients << "\n";
    std::cout << "Requests per client: " << requestsPerClient << "\n";
    std::cout << "Total requests: " << numClients * requestsPerClient << "\n";
    std::cout << "Starting test...\n\n";
    
    auto testStart = std::chrono::steady_clock::now();
    
    std::vector<std::thread> clients;
    for (int i = 0; i < numClients; ++i) {
        clients.emplace_back(clientWorker, host, port, requestsPerClient, i + 1);
    }
    
    for (auto& client : clients) {
        client.join();
    }
    
    auto testEnd = std::chrono::steady_clock::now();
    auto testDuration = std::chrono::duration_cast<std::chrono::milliseconds>(testEnd - testStart);
    
    int successful = successfulRequests.load();
    int failed = failedRequests.load();
    long long totalTime = totalResponseTime.load();
    
    std::cout << "=== Test Results ===\n";
    std::cout << "Test Duration: " << testDuration.count() << "ms\n";
    std::cout << "Successful Requests: " << successful << "\n";
    std::cout << "Failed Requests: " << failed << "\n";
    std::cout << "Success Rate: " << (100.0 * successful / (successful + failed)) << "%\n";
    
    if (successful > 0) {
        std::cout << "Average Response Time: " << (totalTime / successful) << "ms\n";
        std::cout << "Requests per Second: " << (1000.0 * successful / testDuration.count()) << "\n";
    }
    
    return 0;
}