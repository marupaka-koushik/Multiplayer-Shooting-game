#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include "GameState.h"
#include "GameRenderer.h"
#include "InputHandler.h"
#include "NetworkManager.h"

#define SERVER_PORT 8080

class GameClient {
public:
    GameClient() : playerId_(-1), connected_(false) {}
    
    bool initialize(const std::string& serverIP, const std::string& playerName) {
        // Initialize graphics
        if (!renderer_.initialize(800, 600, "Mini Militia Clone")) {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return false;
        }
        
        // Initialize networking
        if (!networkManager_.initializeSocket()) {
            std::cerr << "Failed to initialize networking: " << networkManager_.getLastError() << std::endl;
            return false;
        }
        
        networkManager_.setServerAddress(serverIP, SERVER_PORT);
        playerName_ = playerName;
        
        // Send join request
        NetworkMessage joinMessage;
        joinMessage.type = MessageType::PLAYER_JOIN;
        joinMessage.data = playerName;
        joinMessage.playerId = 0; // Will be assigned by server
        
        networkManager_.sendMessage(joinMessage, networkManager_.getServerAddress());
        
        std::cout << "Connected to server: " << serverIP << ":" << SERVER_PORT << std::endl;
        connected_ = true;
        
        return true;
    }
    
    void run() {
        auto lastUpdate = std::chrono::high_resolution_clock::now();
        
        while (!renderer_.shouldClose() && connected_) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float>(currentTime - lastUpdate).count();
            
            // Cap deltaTime to prevent large jumps
            if (deltaTime > 0.016f) deltaTime = 0.016f;
            
            // Process network messages first to get player ID and game state
            processNetworkMessages();
            
            // Handle input only if we have a valid player ID
            if (playerId_ != -1) {
                inputHandler_.update();
                handleInput();
            }
            
            // Update local game state (prediction)
            gameState_.update(deltaTime);
            
            // Update camera to follow local player
            Player* localPlayer = gameState_.getPlayer(playerId_);
            if (localPlayer) {
                renderer_.updateCamera(*localPlayer);
            }
            
            // Render everything in one go, passing local player ID
            renderer_.render(gameState_, playerId_);
            
            lastUpdate = currentTime;
        }
    }
    
    void cleanup() {
        if (connected_) {
            // Send disconnect message
            NetworkMessage disconnectMessage;
            disconnectMessage.type = MessageType::PLAYER_LEAVE;
            disconnectMessage.playerId = playerId_;
            
            networkManager_.sendMessage(disconnectMessage, networkManager_.getServerAddress());
        }
        
        networkManager_.cleanup();
        renderer_.cleanup();
    }
    
private:
    GameState gameState_;
    GameRenderer renderer_;
    InputHandler inputHandler_;
    NetworkManager networkManager_;
    
    std::string playerName_;
    int playerId_;
    bool connected_;
    
    void handleInput() {
        InputState input = inputHandler_.getInputState();
        Player* localPlayer = gameState_.getPlayer(playerId_);
        
        if (!localPlayer) return;
        
        // Local movement for immediate feedback - 4 directions
        float moveSpeed = 200.0f;
        float velX = 0, velY = 0;
        
        if (input.moveLeft) {
            velX = -moveSpeed;
        }
        if (input.moveRight) {
            velX = moveSpeed;
        }
        
        // Check for up/down movement (W/S keys)
        if (IsKeyDown(KEY_W)) {
            velY = -moveSpeed; // Move up
        }
        if (IsKeyDown(KEY_S)) {
            velY = moveSpeed;  // Move down
        }
        
        // Apply movement locally
        localPlayer->setVelocity(velX, velY);
        
        // Update aim angle based on mouse
        Vector2 playerPos = {localPlayer->getX(), localPlayer->getY()};
        Vector2 mousePos = renderer_.screenToWorld(inputHandler_.getMousePosition());
        float angle = atan2(mousePos.y - playerPos.y, mousePos.x - playerPos.x);
        localPlayer->setAngle(angle);
        
        // Handle shooting
        if (input.shoot) {
            // Send shoot message to server
            NetworkMessage shootMessage;
            shootMessage.type = MessageType::PLAYER_SHOOT;
            shootMessage.playerId = playerId_;
            
            std::ostringstream oss;
            oss << localPlayer->getX() + 10 << "," 
                << localPlayer->getY() + 10 << "," 
                << angle;
            shootMessage.data = oss.str();
            
            networkManager_.sendMessage(shootMessage, networkManager_.getServerAddress());
        }
        
        // Send input to server (for multiplayer sync later)
        if (input.moveLeft || input.moveRight || IsKeyDown(KEY_W) || IsKeyDown(KEY_S)) {
            NetworkMessage moveMessage;
            moveMessage.type = MessageType::PLAYER_MOVE;
            moveMessage.playerId = playerId_;
            
            std::string moveData = "";
            if (input.moveLeft) moveData += "LEFT,";
            if (input.moveRight) moveData += "RIGHT,";
            if (IsKeyDown(KEY_W)) moveData += "UP,";
            if (IsKeyDown(KEY_S)) moveData += "DOWN,";
            
            moveMessage.data = moveData;
            
            networkManager_.sendMessage(moveMessage, networkManager_.getServerAddress());
        }
    }
    
    void processNetworkMessages() {
        NetworkMessage message;
        sockaddr_in fromAddress;
        
        while (networkManager_.receiveMessage(message, fromAddress)) {
            switch (message.type) {
                case MessageType::GAME_STATE_UPDATE:
                    // Update game state from server
                    gameState_.deserialize(message.data);
                    break;
                    
                case MessageType::PLAYER_JOIN:
                    if (playerId_ == -1) {
                        // This is our player ID assignment
                        playerId_ = message.playerId;
                        std::cout << "Assigned player ID: " << playerId_ << std::endl;
                    }
                    break;
                    
                default:
                    break;
            }
        }
    }
};

int main(int argc, char* argv[]) {
    std::string serverIP = "127.0.0.1";
    std::string playerName;
    
    // Get server IP from command line
    if (argc > 1) {
        serverIP = argv[1];
    }
    
    // Get player name
    std::cout << "Enter your name: ";
    std::getline(std::cin, playerName);
    
    if (playerName.empty()) {
        playerName = "Player";
    }
    
    GameClient client;
    
    if (!client.initialize(serverIP, playerName)) {
        return -1;
    }
    
    client.run();
    client.cleanup();
    
    return 0;
}
