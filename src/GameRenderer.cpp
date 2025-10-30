#include "GameRenderer.h"
#include <iostream>

GameRenderer::GameRenderer() 
    : windowWidth_(800), windowHeight_(600), initialized_(false) {
    camera_.target = {0, 0};
    camera_.offset = {windowWidth_ / 2.0f, windowHeight_ / 2.0f};
    camera_.rotation = 0.0f;
    camera_.zoom = 1.0f;
}

GameRenderer::~GameRenderer() {
    cleanup();
}

bool GameRenderer::initialize(int windowWidth, int windowHeight, const std::string& title) {
    windowWidth_ = windowWidth;
    windowHeight_ = windowHeight;
    
    InitWindow(windowWidth_, windowHeight_, title.c_str());
    
    if (!IsWindowReady()) {
        std::cerr << "Failed to initialize window" << std::endl;
        return false;
    }
    
    SetTargetFPS(60);
    
    // Update camera offset
    camera_.offset = {windowWidth_ / 2.0f, windowHeight_ / 2.0f};
    
    loadTextures();
    
    initialized_ = true;
    return true;
}

void GameRenderer::cleanup() {
    if (initialized_) {
        unloadTextures();
        CloseWindow();
        initialized_ = false;
    }
}

void GameRenderer::beginFrame() {
    // This method is now unused - rendering is handled in render()
}

void GameRenderer::endFrame() {
    // This method is now unused - rendering is handled in render()
}

void GameRenderer::render(const GameState& gameState, int localPlayerId) {
    if (!initialized_) return;
    
    // Single BeginDrawing call for entire frame
    BeginDrawing();
    ClearBackground(RAYWHITE);
    
    // World rendering with camera
    BeginMode2D(camera_);
    
    // Render background
    renderBackground();
    
    // Render all players
    const auto& players = gameState.getAllPlayers();
    for (const Player* player : players) {
        if (player && player->isAlive()) {
            bool isLocal = (player->getId() == localPlayerId);
            renderPlayer(*player, isLocal);
        }
    }
    
    // Render all bullets
    const auto& bullets = gameState.getAllBullets();
    for (const Bullet* bullet : bullets) {
        if (bullet && bullet->isActive()) {
            renderBullet(*bullet);
        }
    }
    
    EndMode2D();
    
    // UI rendering (screen space)
    renderUI(gameState);
    
    // Single EndDrawing call
    EndDrawing();
}

void GameRenderer::renderBackground() {
    // Clean background with better colors
    DrawRectangle(0, 0, 800, 600, Color{220, 235, 255, 255}); // Light sky blue
    
    // Subtle grid for visual reference
    for (int x = 0; x < 800; x += 100) {
        DrawLine(x, 0, x, 600, Color{200, 200, 200, 100}); // Light gray, semi-transparent
    }
    for (int y = 0; y < 600; y += 100) {
        DrawLine(0, y, 800, y, Color{200, 200, 200, 100});
    }
    
    // Add some visual landmarks
    DrawRectangle(100, 500, 600, 100, Color{34, 139, 34, 255}); // Ground platform
    DrawRectangle(300, 400, 200, 20, Color{139, 69, 19, 255});  // Small platform
}

void GameRenderer::renderPlayer(const Player& player, bool isLocalPlayer) {
    Vector2 position = {player.getX(), player.getY()};
    
    // Draw player shadow
    DrawEllipse(position.x + 10, position.y + 25, 12, 6, Color{0, 0, 0, 100});
    
    // Draw player body with better graphics
    Color playerColor = isLocalPlayer ? Color{0, 100, 255, 255} : Color{255, 50, 50, 255};
    Color outlineColor = isLocalPlayer ? Color{0, 50, 200, 255} : Color{200, 0, 0, 255};
    
    // Body outline
    DrawRectangle(position.x - 1, position.y - 1, 22, 22, outlineColor);
    // Body fill
    DrawRectangleV(position, {20, 20}, playerColor);
    
    // Draw player direction indicator (gun)
    float angle = player.getAngle();
    Vector2 gunEnd = {position.x + 10 + cos(angle) * 20, 
                      position.y + 10 + sin(angle) * 20};
    DrawLineEx({position.x + 10, position.y + 10}, gunEnd, 3, BLACK);
    
    // Draw player name with background
    std::string playerName = player.getName();
    const char* name = playerName.c_str();
    int textWidth = MeasureText(name, 12);
    DrawRectangle(position.x + 10 - textWidth/2 - 2, position.y - 20, textWidth + 4, 14, Color{0, 0, 0, 150});
    DrawText(name, position.x + 10 - textWidth/2, position.y - 18, 12, WHITE);
    
    // Draw health bar with background
    float healthPercent = (float)player.getHealth() / 100.0f;
    DrawRectangle(position.x - 2, position.y - 10, 24, 6, BLACK);
    DrawRectangle(position.x - 1, position.y - 9, 22, 4, MAROON);
    DrawRectangle(position.x - 1, position.y - 9, 22 * healthPercent, 4, LIME);
}

void GameRenderer::renderBullet(const Bullet& bullet) {
    Vector2 position = {bullet.getX(), bullet.getY()};
    
    // Draw bullet with glow effect
    DrawCircleV(position, 4, Color{255, 255, 0, 100}); // Outer glow
    DrawCircleV(position, 2, Color{255, 255, 100, 255}); // Inner bullet
}

void GameRenderer::renderUI(const GameState& gameState) {
    // Draw UI background panel
    DrawRectangle(5, 5, 200, 80, Color{0, 0, 0, 150});
    DrawRectangleLines(5, 5, 200, 80, WHITE);
    
    // Draw FPS with better styling
    DrawText("FPS:", 15, 15, 16, WHITE);
    DrawText(TextFormat("%d", GetFPS()), 60, 15, 16, LIME);
    
    // Draw player count
    const auto& players = gameState.getAllPlayers();
    DrawText("Players:", 15, 35, 16, WHITE);
    DrawText(TextFormat("%d", (int)players.size()), 85, 35, 16, SKYBLUE);
    
    // Draw bullet count
    const auto& bullets = gameState.getAllBullets();
    DrawText("Bullets:", 15, 55, 16, WHITE);
    DrawText(TextFormat("%d", (int)bullets.size()), 85, 55, 16, ORANGE);
    
    // Draw controls help
    DrawText("Controls: A/D - Move | W/S - Up/Down | Mouse - Aim/Shoot", 10, windowHeight_ - 25, 14, WHITE);
    DrawRectangle(5, windowHeight_ - 30, 520, 25, Color{0, 0, 0, 100});
}

void GameRenderer::renderHUD(const Player* localPlayer) {
    if (!localPlayer) return;
    
    // Draw health
    std::string healthText = "Health: " + std::to_string(localPlayer->getHealth());
    DrawText(healthText.c_str(), windowWidth_ - 150, 10, 20, RED);
    
    // Draw crosshair
    Vector2 mousePos = GetMousePosition();
    DrawCircleLines(mousePos.x, mousePos.y, 10, RED);
    DrawLine(mousePos.x - 5, mousePos.y, mousePos.x + 5, mousePos.y, RED);
    DrawLine(mousePos.x, mousePos.y - 5, mousePos.x, mousePos.y + 5, RED);
}

bool GameRenderer::shouldClose() const {
    return WindowShouldClose();
}

Vector2 GameRenderer::getMousePosition() const {
    return GetMousePosition();
}

Vector2 GameRenderer::screenToWorld(Vector2 screenPos) const {
    return GetScreenToWorld2D(screenPos, camera_);
}

Vector2 GameRenderer::worldToScreen(Vector2 worldPos) const {
    return GetWorldToScreen2D(worldPos, camera_);
}

void GameRenderer::setCameraTarget(float x, float y) {
    camera_.target = {x, y};
}

void GameRenderer::updateCamera(const Player& player) {
    // Follow the player
    camera_.target = {player.getX(), player.getY()};
}

void GameRenderer::loadTextures() {
    // For now, we'll use simple shapes instead of textures
    // You can load actual textures here:
    // playerTexture_ = LoadTexture("assets/player.png");
    // gunTexture_ = LoadTexture("assets/gun.png");
}

void GameRenderer::unloadTextures() {
    // Unload textures when they're actually loaded:
    // UnloadTexture(playerTexture_);
    // UnloadTexture(gunTexture_);
}