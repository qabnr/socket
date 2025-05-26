#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

int main() {
/*
    const char* server_ip = "127.0.0.1";
/*/
    const char* server_ip = "192.168.1.88";
/**/
    const int server_port = 8080;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address\n";
        close(sock);
        return 1;
    }

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed\n";
        close(sock);
        return 1;
    }

    const char* msg = "Hello from client";
    int counter = 0;
    while (true) {
        send(sock, msg, strlen(msg), 0);

        char buffer[1024] = {0};
        int valread = read(sock, buffer, sizeof(buffer) - 1);
        if (valread <= 0) {
            std::cerr << "Server disconnected\n";
            break;
        }
        if (valread > 0) {
            std::cout << "Server: " << buffer << " [#" << ++counter << "]" << std::endl;
        }
        if (strcmp(buffer, "exit") == 0) {
            std::cout << "Server closed the connection\n";
            break;
        }
        sleep(5);
    }
    close(sock);
    return 0;
}
