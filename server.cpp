#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    bind(server_fd, (const struct sockaddr*)&server_addr, sizeof(server_addr));
    
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        
        ssize_t bytes_received = recvfrom(server_fd, buffer, BUFFER_SIZE - 1, 0,
                                        (struct sockaddr*)&client_addr, &client_len);
        
        if (bytes_received > 0) {
            sendto(server_fd, buffer, bytes_received, 0,
                  (const struct sockaddr*)&client_addr, client_len);
        }
    }
    
    close(server_fd);
    return 0;
}
