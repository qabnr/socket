#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

int main() {
    const int port = 8080;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed\n";
        return 1;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    address.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        std::cerr << "Listen failed\n";
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port " << port << std::endl;

    // Reap zombie children
    signal(SIGCHLD, [](int){ while (waitpid(-1, nullptr, WNOHANG) > 0); });

    int client_cnt = 0;
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_sock = accept(server_fd, (sockaddr*)&client_addr, &client_len);
        if (client_sock < 0) {
            std::cerr << "Accept failed\n";
            continue;
        }

        int client_id = ++client_cnt;
        pid_t pid = fork();
        if (pid < 0) {
            std::cerr << "Fork failed\n";
            close(client_sock);
            continue;
        }
        if (pid == 0) { // Child process
            close(server_fd); // Child doesn't need the listening socket
            int cnt = 0;
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            std::cout << "Client connected: id=" << client_id << ", ip=" << client_ip << ", port=" << ntohs(client_addr.sin_port) << std::endl;
            while (true)
            {
                char buffer[1024] = {0};
                int valread = read(client_sock, buffer, sizeof(buffer) - 1);
                if (valread > 0) {
                    std::cout << "Client[" << client_id << "]: " << buffer << " (" << ++cnt << ")\n";
                    const char* reply = "Hello from server";
                    send(client_sock, reply, strlen(reply), 0);
                }
                if (strcmp(buffer, "exit") == 0) {
                    std::cout << "Client[" << client_id << "] closed the connection\n";
                    break;
                }
                if (valread <= 0) {
                    std::cerr << "Client[" << client_id << "] disconnected\n";
                    break;
                }
            }
            close(client_sock);
            return 0;
        } else {
            // Parent process
            close(client_sock); // Parent doesn't need this socket
        }
    }
    close(server_fd);
    return 0;
}
