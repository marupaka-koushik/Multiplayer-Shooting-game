#pragma once
#include "raylib.h"
#include "GameState.h"
#include "NetworkManager.h"

class GameRenderer {
public:
    GameRenderer();
    ~GameRenderer();
    
    // Initialization
    bool initialize(int windowWidth, int windowHeight, const std::string& title);
    void cleanup();
    
    // Main rendering
    void beginFrame();
    void endFrame();
    void render(const GameState& gameState);
    
    // Individual rendering functions
    void renderBackground();
    void renderPlayer(const Player& player, bool isLocalPlayer = false);
    void renderBullet(const Bullet& bullet);
    void renderUI(const GameState& gameState);
    void renderHUD(const Player* localPlayer);
    
    // Utility
    bool shouldClose() const;
    Vector2 getMousePosition() const;
    Vector2 screenToWorld(Vector2 screenPos) const;
    Vector2 worldToScreen(Vector2 worldPos) const;
    
    // Camera
    void setCameraTarget(float x, float y);
    void updateCamera(const Player& player);
    
private:
    int windowWidth_;
    int windowHeight_;
    Camera2D camera_;
    bool initialized_;
    
    // Textures (will be loaded from assets)
    Texture2D playerTexture_;
    Texture2D gunTexture_;
    
    void loadTextures();
    void unloadTextures();
};