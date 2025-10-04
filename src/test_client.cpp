#include <iostream>
#include <string>

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

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <host> <port>\n";
        return 1;
    }

#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2,2), &wsaData);
#endif

    std::string host = argv[1];
    int port = std::stoi(argv[2]);

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);

    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        std::cout << "Connection failed\n";
        return 1;
    }

    std::string request = "GET / HTTP/1.0\r\nHost: " + host + "\r\n\r\n";
    send(clientSocket, request.c_str(), request.length(), 0);

    char buffer[4096];
    std::string response;
    int bytesReceived;
    
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        response += buffer;
    }

    std::cout << "Response:\n" << response << std::endl;
    closesocket(clientSocket);

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
