#include "NetworkManager.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <sstream>

NetworkManager::NetworkManager() : socket_(-1), initialized_(false) {
    memset(&serverAddr_, 0, sizeof(serverAddr_));
}

NetworkManager::~NetworkManager() {
    cleanup();
}

bool NetworkManager::initializeSocket() {
    socket_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ < 0) {
        setError("Failed to create socket");
        return false;
    }
    
    initialized_ = true;
    return true;
}

void NetworkManager::cleanup() {
    if (socket_ >= 0) {
        close(socket_);
        socket_ = -1;
    }
    initialized_ = false;
}

bool NetworkManager::sendMessage(const NetworkMessage& message, const sockaddr_in& address) {
    if (!initialized_) {
        setError("Network manager not initialized");
        return false;
    }
    
    std::string serialized = message.serialize();
    ssize_t bytesSent = sendto(socket_, serialized.c_str(), serialized.length(), 0,
                              (const sockaddr*)&address, sizeof(address));
    
    if (bytesSent < 0) {
        setError("Failed to send message");
        return false;
    }
    
    return true;
}

bool NetworkManager::receiveMessage(NetworkMessage& message, sockaddr_in& fromAddress) {
    if (!initialized_) {
        setError("Network manager not initialized");
        return false;
    }
    
    char buffer[1024];
    socklen_t fromLen = sizeof(fromAddress);
    
    ssize_t bytesReceived = recvfrom(socket_, buffer, sizeof(buffer) - 1, MSG_DONTWAIT,
                                    (sockaddr*)&fromAddress, &fromLen);
    
    if (bytesReceived < 0) {
        // No data available (non-blocking)
        return false;
    }
    
    buffer[bytesReceived] = '\0';
    message = NetworkMessage::deserialize(std::string(buffer));
    
    return true;
}

bool NetworkManager::bindToPort(int port) {
    if (!initialized_) {
        setError("Network manager not initialized");
        return false;
    }
    
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(socket_, (const sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        setError("Failed to bind to port");
        return false;
    }
    
    return true;
}

bool NetworkManager::startListening() {
    // For UDP, no explicit listen needed
    return initialized_;
}

bool NetworkManager::connectToServer(const std::string& serverIP, int port) {
    setServerAddress(serverIP, port);
    return true; // UDP doesn't require explicit connection
}

void NetworkManager::setServerAddress(const std::string& serverIP, int port) {
    memset(&serverAddr_, 0, sizeof(serverAddr_));
    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_port = htons(port);
    serverAddr_.sin_addr.s_addr = inet_addr(serverIP.c_str());
}

void NetworkManager::setError(const std::string& error) {
    lastError_ = error;
}

// NetworkMessage implementation
std::string NetworkMessage::serialize() const {
    std::ostringstream oss;
    oss << static_cast<int>(type) << "|" << playerId << "|" << data;
    return oss.str();
}

NetworkMessage NetworkMessage::deserialize(const std::string& data) {
    NetworkMessage message;
    
    size_t firstPipe = data.find('|');
    size_t secondPipe = data.find('|', firstPipe + 1);
    
    if (firstPipe != std::string::npos && secondPipe != std::string::npos) {
        message.type = static_cast<MessageType>(std::stoi(data.substr(0, firstPipe)));
        message.playerId = std::stoi(data.substr(firstPipe + 1, secondPipe - firstPipe - 1));
        message.data = data.substr(secondPipe + 1);
    }
    
    return message;
}