#pragma once
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>

enum class MessageType {
    PLAYER_JOIN,
    PLAYER_LEAVE,
    PLAYER_MOVE,
    PLAYER_SHOOT,
    PLAYER_RESPAWN,
    GAME_STATE_UPDATE,
    PING,
    PONG
};

struct NetworkMessage {
    MessageType type;
    std::string data;
    int playerId;
    
    std::string serialize() const;
    static NetworkMessage deserialize(const std::string& data);
};

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();
    
    // Common networking
    bool initializeSocket();
    void cleanup();
    bool sendMessage(const NetworkMessage& message, const sockaddr_in& address);
    bool receiveMessage(NetworkMessage& message, sockaddr_in& fromAddress);
    
    // Server specific
    bool bindToPort(int port);
    bool startListening();
    
    // Client specific  
    bool connectToServer(const std::string& serverIP, int port);
    void setServerAddress(const std::string& serverIP, int port);
    sockaddr_in getServerAddress() const { return serverAddr_; }
    
    // Utility
    std::string getLastError() const { return lastError_; }
    bool isInitialized() const { return initialized_; }
    
private:
    int socket_;
    sockaddr_in serverAddr_;
    bool initialized_;
    std::string lastError_;
    
    void setError(const std::string& error);
};