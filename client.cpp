#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    int client_fd;
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);
    char buffer[BUFFER_SIZE];
    std::string message;
    
    std::string server_ip = "127.0.0.1";
    if (argc > 1) {
        server_ip = argv[1];
    }
    
    client_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
    
    while (true) {
        std::getline(std::cin, message);
        
        if (message == "quit") {
            break;
        }
        
        sendto(client_fd, message.c_str(), message.length(), 0,
               (const struct sockaddr*)&server_addr, server_len);
        
        memset(buffer, 0, BUFFER_SIZE);
        recvfrom(client_fd, buffer, BUFFER_SIZE - 1, 0,
                (struct sockaddr*)&server_addr, &server_len);
    }
    
    close(client_fd);
    return 0;
}
