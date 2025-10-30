#include <iostream>
#include <map>
#include <chrono>
#include <thread>
#include <sstream>
#include "GameState.h"
#include "NetworkManager.h"

#define PORT 8080
#define TICK_RATE 30 // 30 FPS server tick rate

class GameServer {
public:
    GameServer() : running_(false), nextPlayerId_(1) {}
    
    bool initialize() {
        if (!networkManager_.initializeSocket()) {
            std::cerr << "Failed to initialize socket: " << networkManager_.getLastError() << std::endl;
            return false;
        }
        
        if (!networkManager_.bindToPort(PORT)) {
            std::cerr << "Failed to bind to port: " << networkManager_.getLastError() << std::endl;
            return false;
        }
        
        gameState_.setWorldSize(800, 600);
        
        std::cout << "Game server initialized on port " << PORT << std::endl;
        return true;
    }
    
    void run() {
        running_ = true;
        
        const float tickTime = 1.0f / TICK_RATE;
        auto lastTick = std::chrono::high_resolution_clock::now();
        
        while (running_) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastTick).count();
            
            if (deltaTime >= tickTime) {
                processMessages();
                gameState_.update(deltaTime);
                broadcastGameState();
                
                lastTick = currentTime;
            }
            
            // Small sleep to prevent 100% CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
    
    void stop() {
        running_ = false;
    }
    
private:
    GameState gameState_;
    NetworkManager networkManager_;
    std::map<int, sockaddr_in> clientAddresses_;
    bool running_;
    int nextPlayerId_;
    
    void processMessages() {
        NetworkMessage message;
        sockaddr_in fromAddress;
        
        while (networkManager_.receiveMessage(message, fromAddress)) {
            handleMessage(message, fromAddress);
        }
    }
    
    void handleMessage(const NetworkMessage& message, const sockaddr_in& fromAddress) {
        switch (message.type) {
            case MessageType::PLAYER_JOIN: {
                int playerId = nextPlayerId_++;
                gameState_.addPlayer(playerId, message.data);
                clientAddresses_[playerId] = fromAddress;
                
                // Send player ID assignment back to the client
                NetworkMessage assignMessage;
                assignMessage.type = MessageType::PLAYER_JOIN;
                assignMessage.playerId = playerId;
                assignMessage.data = message.data; // player name
                networkManager_.sendMessage(assignMessage, fromAddress);
                
                std::cout << "Player " << message.data << " joined (ID: " << playerId << ")" << std::endl;
                std::cout << "Total players: " << clientAddresses_.size() << std::endl;
                break;
            }
            case MessageType::PLAYER_MOVE: {
                Player* player = gameState_.getPlayer(message.playerId);
                if (player) {
                    // Parse movement data: "LEFT,RIGHT,UP,DOWN,"
                    std::string moveData = message.data;
                    float velX = 0, velY = 0;
                    float moveSpeed = 200.0f;
                    
                    if (moveData.find("LEFT") != std::string::npos) {
                        velX = -moveSpeed;
                    }
                    if (moveData.find("RIGHT") != std::string::npos) {
                        velX = moveSpeed;
                    }
                    if (moveData.find("UP") != std::string::npos) {
                        velY = -moveSpeed;
                    }
                    if (moveData.find("DOWN") != std::string::npos) {
                        velY = moveSpeed;
                    }
                    
                    player->setVelocity(velX, velY);
                }
                break;
            }
            case MessageType::PLAYER_SHOOT: {
                // Handle shooting
                // Format: "x,y,angle"
                Player* player = gameState_.getPlayer(message.playerId);
                if (player) {
                    // Parse shooting data
                    std::istringstream iss(message.data);
                    std::string token;
                    float x, y, angle;
                    
                    std::getline(iss, token, ',');
                    x = std::stof(token);
                    std::getline(iss, token, ',');
                    y = std::stof(token);
                    std::getline(iss, token, ',');
                    angle = std::stof(token);
                    
                    static int bulletIdCounter = 1;
                    gameState_.addBullet(bulletIdCounter++, message.playerId, x, y, angle, 400.0f);
                }
                break;
            }
            case MessageType::PLAYER_LEAVE: {
                gameState_.removePlayer(message.playerId);
                clientAddresses_.erase(message.playerId);
                std::cout << "Player " << message.playerId << " left" << std::endl;
                std::cout << "Total players: " << clientAddresses_.size() << std::endl;
                break;
            }
            default:
                break;
        }
    }
    
    void broadcastGameState() {
        std::string gameStateData = gameState_.serialize();
        NetworkMessage message;
        message.type = MessageType::GAME_STATE_UPDATE;
        message.data = gameStateData;
        message.playerId = 0; // Server message
        
        // Send to all connected clients
        for (const auto& client : clientAddresses_) {
            networkManager_.sendMessage(message, client.second);
        }
    }
};

int main() {
    GameServer server;
    
    if (!server.initialize()) {
        return -1;
    }
    
    server.run();
    
    return 0;
}
