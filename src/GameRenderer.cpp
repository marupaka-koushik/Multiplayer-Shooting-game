#include "GameRenderer.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <vector>

GameRenderer::GameRenderer() 
    : windowWidth_(800), windowHeight_(600), initialized_(false) {
    // Initialize camera to center of world
    camera_.target = {400, 300};
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
    
    // Render leaderboard
    renderLeaderboard(gameState);
    
    // Check if local player is dead and show respawn message
    const Player* localPlayer = nullptr;
    for (const Player* player : players) {
        if (player && player->getId() == localPlayerId) {
            localPlayer = player;
            break;
        }
    }
    
    if (localPlayer && !localPlayer->isAlive()) {
        // Draw semi-transparent overlay
        DrawRectangle(0, 0, windowWidth_, windowHeight_, Color{0, 0, 0, 150});
        
        // Draw death message
        const char* deathMsg = "YOU DIED";
        int deathMsgWidth = MeasureText(deathMsg, 60);
        DrawText(deathMsg, windowWidth_/2 - deathMsgWidth/2, windowHeight_/2 - 60, 60, RED);
        
        // Draw respawn instruction
        const char* respawnMsg = "Press R to Respawn";
        int respawnMsgWidth = MeasureText(respawnMsg, 30);
        DrawText(respawnMsg, windowWidth_/2 - respawnMsgWidth/2, windowHeight_/2 + 20, 30, WHITE);
    }
    
    // Single EndDrawing call
    EndDrawing();
}

void GameRenderer::renderBackground() {
    // Calculate visible world bounds based on camera
    Vector2 topLeft = GetScreenToWorld2D({0, 0}, camera_);
    Vector2 bottomRight = GetScreenToWorld2D({(float)windowWidth_, (float)windowHeight_}, camera_);
    
    // World boundaries (matching server's 800x600)
    const float worldWidth = 800.0f;
    const float worldHeight = 600.0f;
    
    // Draw out-of-bounds area (darker/different color) to show it's not playable
    DrawRectangle(topLeft.x - 100, topLeft.y - 100, 
                  bottomRight.x - topLeft.x + 200, 
                  bottomRight.y - topLeft.y + 200, 
                  Color{40, 40, 60, 255}); // Dark blue-gray for out of bounds
    
    // Draw the playable map area with sky background
    DrawRectangle(0, 0, worldWidth, worldHeight, Color{220, 235, 255, 255}); // Light sky blue
    
    // Draw grid for visual reference within the playable world only
    int gridSize = 100;
    for (int x = 0; x <= (int)worldWidth; x += gridSize) {
        DrawLine(x, 0, x, worldHeight, Color{200, 200, 200, 100});
    }
    for (int y = 0; y <= (int)worldHeight; y += gridSize) {
        DrawLine(0, y, worldWidth, y, Color{200, 200, 200, 100});
    }
    
    // Draw thick boundary walls around the map
    int wallThickness = 10;
    
    // Top wall
    DrawRectangle(0, -wallThickness, worldWidth, wallThickness, Color{100, 100, 100, 255});
    DrawRectangle(0, 0, worldWidth, 3, Color{60, 60, 60, 255}); // Inner shadow
    
    // Bottom wall
    DrawRectangle(0, worldHeight, worldWidth, wallThickness, Color{100, 100, 100, 255});
    DrawRectangle(0, worldHeight - 3, worldWidth, 3, Color{180, 180, 180, 255}); // Inner highlight
    
    // Left wall
    DrawRectangle(-wallThickness, 0, wallThickness, worldHeight, Color{100, 100, 100, 255});
    DrawRectangle(0, 0, 3, worldHeight, Color{60, 60, 60, 255}); // Inner shadow
    
    // Right wall
    DrawRectangle(worldWidth, 0, wallThickness, worldHeight, Color{100, 100, 100, 255});
    DrawRectangle(worldWidth - 3, 0, 3, worldHeight, Color{180, 180, 180, 255}); // Inner highlight
    
    // Draw corner markers for extra visibility
    int cornerSize = 20;
    // Top-left corner
    DrawRectangle(0, 0, cornerSize, cornerSize, Color{255, 0, 0, 180});
    // Top-right corner
    DrawRectangle(worldWidth - cornerSize, 0, cornerSize, cornerSize, Color{255, 0, 0, 180});
    // Bottom-left corner
    DrawRectangle(0, worldHeight - cornerSize, cornerSize, cornerSize, Color{255, 0, 0, 180});
    // Bottom-right corner
    DrawRectangle(worldWidth - cornerSize, worldHeight - cornerSize, cornerSize, cornerSize, Color{255, 0, 0, 180});
    
    // Add some visual landmarks (these are in world coordinates)
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
    DrawCircleV(position, 4, Color{255, 0, 0, 100}); // Outer glow (red)
    DrawCircleV(position, 2, Color{255, 50, 50, 255}); // Inner bullet (red)
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

void GameRenderer::renderLeaderboard(const GameState& gameState) {
    const auto& players = gameState.getAllPlayers();
    if (players.empty()) return;
    
    // Create a sorted list of players by kills
    std::vector<const Player*> sortedPlayers(players.begin(), players.end());
    std::sort(sortedPlayers.begin(), sortedPlayers.end(), 
              [](const Player* a, const Player* b) {
                  return a->getKills() > b->getKills();
              });
    
    // Leaderboard position and size
    int boardX = windowWidth_ - 220;
    int boardY = 50;
    int boardWidth = 210;
    int maxEntries = 5;
    int entryHeight = 25;
    int headerHeight = 30;
    int boardHeight = headerHeight + (std::min((int)sortedPlayers.size(), maxEntries) * entryHeight) + 10;
    
    // Draw leaderboard background
    DrawRectangle(boardX, boardY, boardWidth, boardHeight, Color{0, 0, 0, 180});
    DrawRectangleLines(boardX, boardY, boardWidth, boardHeight, Color{255, 215, 0, 255}); // Gold border
    
    // Draw header
    const char* title = "LEADERBOARD";
    int titleWidth = MeasureText(title, 18);
    DrawText(title, boardX + (boardWidth - titleWidth) / 2, boardY + 5, 18, Color{255, 215, 0, 255});
    
    // Draw column headers
    DrawText("Player", boardX + 10, boardY + headerHeight, 14, Color{200, 200, 200, 255});
    DrawText("K", boardX + 130, boardY + headerHeight, 14, Color{100, 255, 100, 255});
    DrawText("D", boardX + 160, boardY + headerHeight, 14, Color{255, 100, 100, 255});
    
    // Draw top players
    int yPos = boardY + headerHeight + 20;
    for (int i = 0; i < std::min(maxEntries, (int)sortedPlayers.size()); i++) {
        const Player* player = sortedPlayers[i];
        
        // Rank color
        Color rankColor = WHITE;
        if (i == 0) rankColor = Color{255, 215, 0, 255}; // Gold
        else if (i == 1) rankColor = Color{192, 192, 192, 255}; // Silver
        else if (i == 2) rankColor = Color{205, 127, 50, 255}; // Bronze
        
        // Draw rank number
        DrawText(TextFormat("%d.", i + 1), boardX + 5, yPos, 14, rankColor);
        
        // Draw player name (truncate if too long)
        std::string playerName = player->getName();
        if (playerName.length() > 10) {
            playerName = playerName.substr(0, 9) + "..";
        }
        DrawText(playerName.c_str(), boardX + 25, yPos, 14, WHITE);
        
        // Draw kills
        DrawText(TextFormat("%d", player->getKills()), boardX + 130, yPos, 14, Color{100, 255, 100, 255});
        
        // Draw deaths
        DrawText(TextFormat("%d", player->getDeaths()), boardX + 160, yPos, 14, Color{255, 100, 100, 255});
        
        yPos += entryHeight;
    }
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