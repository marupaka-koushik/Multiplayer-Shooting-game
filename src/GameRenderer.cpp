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
    // World boundaries (2.5x bigger: 2000x1500)
    const float worldWidth = 2000.0f;
    const float worldHeight = 1500.0f;
    
    // Draw the playable map area with sky background
    DrawRectangle(0, 0, worldWidth, worldHeight, Color{135, 206, 235, 255}); // Sky blue
    
    // Draw subtle grid for visual reference (50 unit grid)
    int gridSize = 50;
    for (int x = 0; x < (int)worldWidth; x += gridSize) {
        DrawLine(x, 0, x, worldHeight, Color{180, 200, 220, 60});
    }
    for (int y = 0; y < (int)worldHeight; y += gridSize) {
        DrawLine(0, y, worldWidth, y, Color{180, 200, 220, 60});
    }
    
    // Draw thicker boundary walls
    int wallThickness = 25;
    
    // Top wall
    DrawRectangle(0, -wallThickness, worldWidth, wallThickness, Color{80, 80, 80, 255});
    
    // Bottom wall
    DrawRectangle(0, worldHeight, worldWidth, wallThickness, Color{80, 80, 80, 255});
    
    // Left wall
    DrawRectangle(-wallThickness, 0, wallThickness, worldHeight, Color{80, 80, 80, 255});
    
    // Right wall
    DrawRectangle(worldWidth, 0, wallThickness, worldHeight, Color{80, 80, 80, 255});
    
    // ===== MAP STRUCTURES (Mini Militia style with lots of obstacles) =====
    
    // Ground floor
    DrawRectangle(0, 1375, worldWidth, 125, Color{34, 139, 34, 255}); // Grass
    DrawRectangle(0, 1362, worldWidth, 13, Color{60, 179, 113, 255}); // Grass highlight
    
    // === CENTRAL AREA ===
    // Central building/bunker
    DrawRectangle(812, 1000, 375, 375, Color{120, 120, 120, 255}); // Main structure
    DrawRectangle(812, 1000, 375, 13, Color{160, 160, 160, 255}); // Top highlight
    DrawRectangle(812, 1000, 13, 375, Color{80, 80, 80, 255}); // Left shadow
    DrawRectangle(1000, 1125, 75, 125, Color{60, 60, 60, 255}); // Door
    DrawRectangle(850, 1050, 50, 50, Color{100, 150, 200, 255}); // Window
    DrawRectangle(1075, 1050, 50, 50, Color{100, 150, 200, 255}); // Window
    
    // Central vertical walls for cover
    DrawRectangle(950, 700, 100, 200, Color{100, 100, 100, 255});
    DrawRectangle(950, 700, 100, 10, Color{130, 130, 130, 255});
    
    // === LEFT SIDE STRUCTURES ===
    // Left corner bunker
    DrawRectangle(50, 1150, 200, 225, Color{139, 69, 19, 255});
    DrawRectangle(50, 1150, 200, 10, Color{180, 100, 30, 255});
    DrawRectangle(100, 1200, 50, 75, Color{60, 60, 60, 255}); // Door
    
    // Left mid platforms
    DrawRectangle(125, 875, 300, 50, Color{139, 69, 19, 255});
    DrawRectangle(125, 862, 300, 13, Color{180, 100, 30, 255});
    DrawRectangle(200, 925, 38, 450, Color{101, 67, 33, 255}); // Support pillar left
    DrawRectangle(312, 925, 38, 450, Color{101, 67, 33, 255}); // Support pillar right
    
    // Left upper platform
    DrawRectangle(375, 500, 250, 38, Color{128, 128, 128, 255});
    DrawRectangle(375, 487, 250, 13, Color{160, 160, 160, 255});
    
    // Left side walls and obstacles
    DrawRectangle(250, 1100, 150, 50, Color{128, 128, 128, 255}); // Horizontal wall
    DrawRectangle(150, 600, 50, 200, Color{100, 100, 100, 255}); // Vertical wall
    DrawRectangle(450, 750, 50, 150, Color{100, 100, 100, 255}); // Vertical wall
    
    // === RIGHT SIDE STRUCTURES ===
    // Right corner bunker
    DrawRectangle(1750, 1150, 200, 225, Color{139, 69, 19, 255});
    DrawRectangle(1750, 1150, 200, 10, Color{180, 100, 30, 255});
    DrawRectangle(1850, 1200, 50, 75, Color{60, 60, 60, 255}); // Door
    
    // Right mid platforms
    DrawRectangle(1575, 875, 300, 50, Color{139, 69, 19, 255});
    DrawRectangle(1575, 862, 300, 13, Color{180, 100, 30, 255});
    DrawRectangle(1650, 925, 38, 450, Color{101, 67, 33, 255}); // Support pillar left
    DrawRectangle(1762, 925, 38, 450, Color{101, 67, 33, 255}); // Support pillar right
    
    // Right upper platform
    DrawRectangle(1375, 500, 250, 38, Color{128, 128, 128, 255});
    DrawRectangle(1375, 487, 250, 13, Color{160, 160, 160, 255});
    
    // Right side walls and obstacles
    DrawRectangle(1600, 1100, 150, 50, Color{128, 128, 128, 255}); // Horizontal wall
    DrawRectangle(1800, 600, 50, 200, Color{100, 100, 100, 255}); // Vertical wall
    DrawRectangle(1500, 750, 50, 150, Color{100, 100, 100, 255}); // Vertical wall
    
    // === TOP AREA ===
    // Top center platform
    DrawRectangle(875, 300, 250, 50, Color{160, 82, 45, 255});
    DrawRectangle(875, 287, 250, 13, Color{205, 133, 63, 255});
    
    // Top left and right platforms
    DrawRectangle(200, 200, 200, 40, Color{128, 128, 128, 255});
    DrawRectangle(1600, 200, 200, 40, Color{128, 128, 128, 255});
    
    // Top floating obstacles
    DrawRectangle(600, 400, 80, 80, Color{100, 100, 100, 255});
    DrawRectangle(1320, 400, 80, 80, Color{100, 100, 100, 255});
    
    // === MIDDLE AREA OBSTACLES ===
    // Scattered cover boxes/crates
    // Left-center crates
    DrawRectangle(450, 1300, 100, 75, Color{139, 90, 43, 255});
    DrawRectangle(450, 1300, 100, 8, Color{180, 120, 60, 255});
    DrawRectangle(575, 1275, 88, 100, Color{139, 90, 43, 255});
    DrawRectangle(575, 1275, 88, 8, Color{180, 120, 60, 255});
    
    // Right-center crates
    DrawRectangle(1300, 1300, 100, 75, Color{139, 90, 43, 255});
    DrawRectangle(1300, 1300, 100, 8, Color{180, 120, 60, 255});
    DrawRectangle(1425, 1275, 88, 100, Color{139, 90, 43, 255});
    DrawRectangle(1425, 1275, 88, 8, Color{180, 120, 60, 255});
    
    // Mid-level scattered crates
    DrawRectangle(250, 1000, 75, 75, Color{139, 90, 43, 255});
    DrawRectangle(1200, 1300, 75, 75, Color{139, 90, 43, 255});
    DrawRectangle(750, 1300, 75, 75, Color{139, 90, 43, 255});
    
    // Small obstacles for tactical cover
    DrawRectangle(700, 950, 60, 60, Color{100, 100, 100, 255});
    DrawRectangle(1240, 950, 60, 60, Color{100, 100, 100, 255});
    DrawRectangle(500, 650, 60, 60, Color{100, 100, 100, 255});
    DrawRectangle(1440, 650, 60, 60, Color{100, 100, 100, 255});
    
    // === ADDITIONAL PLATFORMS ===
    // Lower mid platforms
    DrawRectangle(250, 1100, 200, 30, Color{128, 128, 128, 255});
    DrawRectangle(1550, 1100, 200, 30, Color{128, 128, 128, 255});
    
    // Diagonal cover walls
    DrawRectangle(350, 550, 40, 250, Color{120, 120, 120, 255});
    DrawRectangle(1610, 550, 40, 250, Color{120, 120, 120, 255});
    
    // Center-left and center-right vertical obstacles
    DrawRectangle(650, 800, 50, 150, Color{100, 100, 100, 255});
    DrawRectangle(1300, 800, 50, 150, Color{100, 100, 100, 255});
    
    // Small floating platforms for vertical gameplay
    DrawRectangle(100, 400, 100, 30, Color{139, 69, 19, 255});
    DrawRectangle(1800, 400, 100, 30, Color{139, 69, 19, 255});
    DrawRectangle(500, 250, 120, 30, Color{139, 69, 19, 255});
    DrawRectangle(1380, 250, 120, 30, Color{139, 69, 19, 255});
    
    // Top corners cover
    DrawRectangle(50, 50, 100, 100, Color{120, 120, 120, 255});
    DrawRectangle(1850, 50, 100, 100, Color{120, 120, 120, 255});
    
    // Bottom corners obstacles
    DrawRectangle(100, 1250, 80, 80, Color{139, 90, 43, 255});
    DrawRectangle(1820, 1250, 80, 80, Color{139, 90, 43, 255});
    
    // === MORE MID-LEVEL OBSTACLES ===
    // Horizontal cover walls at different heights
    DrawRectangle(800, 600, 150, 40, Color{120, 120, 120, 255});
    DrawRectangle(1050, 600, 150, 40, Color{120, 120, 120, 255});
    
    // Additional small platforms
    DrawRectangle(300, 750, 100, 25, Color{139, 69, 19, 255});
    DrawRectangle(1600, 750, 100, 25, Color{139, 69, 19, 255});
    
    // More tactical crates
    DrawRectangle(900, 1150, 70, 70, Color{139, 90, 43, 255});
    DrawRectangle(1030, 1150, 70, 70, Color{139, 90, 43, 255});
    
    // Corner markers for visibility
    int cornerSize = 50;
    DrawRectangle(0, 0, cornerSize, cornerSize, Color{255, 0, 0, 180});
    DrawRectangle(worldWidth - cornerSize, 0, cornerSize, cornerSize, Color{255, 0, 0, 180});
}

void GameRenderer::renderPlayer(const Player& player, bool isLocalPlayer) {
    Vector2 position = {player.getX(), player.getY()};
    
    // Draw player shadow (adjusted for larger player)
    DrawEllipse(position.x + 20, position.y + 45, 20, 10, Color{0, 0, 0, 100});
    
    // Draw player using texture if loaded, otherwise use colored rectangle
    if (playerTexture_.id != 0) {
        // Tint color based on whether it's the local player
        Color tint = isLocalPlayer ? Color{100, 150, 255, 255} : WHITE;
        
        // Draw player texture centered (increased size from 20x20 to 40x40)
        Rectangle source = {0, 0, (float)playerTexture_.width, (float)playerTexture_.height};
        Rectangle dest = {position.x + 20, position.y + 20, 40, 40};
        Vector2 origin = {20, 20};
        
        DrawTexturePro(playerTexture_, source, dest, origin, 0, tint);
    } else {
        // Fallback to colored rectangles (increased size)
        Color playerColor = isLocalPlayer ? Color{0, 100, 255, 255} : Color{255, 50, 50, 255};
        Color outlineColor = isLocalPlayer ? Color{0, 50, 200, 255} : Color{200, 0, 0, 255};
        
        DrawRectangle(position.x - 1, position.y - 1, 42, 42, outlineColor);
        DrawRectangleV(position, {40, 40}, playerColor);
    }
    
    // Draw gun using texture if loaded
    float angle = player.getAngle();
    if (gunTexture_.id != 0) {
        // Draw gun texture rotated (adjusted for larger player)
        Rectangle source = {0, 0, (float)gunTexture_.width, (float)gunTexture_.height};
        Rectangle dest = {position.x + 20, position.y + 20, 30, 12};
        Vector2 origin = {0, 6};
        
        DrawTexturePro(gunTexture_, source, dest, origin, angle * RAD2DEG, WHITE);
    } else {
        // Fallback to line (adjusted for larger player)
        Vector2 gunEnd = {position.x + 20 + cos(angle) * 30, 
                          position.y + 20 + sin(angle) * 30};
        DrawLineEx({position.x + 20, position.y + 20}, gunEnd, 3, BLACK);
    }
    
    // Draw player name with background
    std::string playerName = player.getName();
    const char* name = playerName.c_str();
    int textWidth = MeasureText(name, 12);
    DrawRectangle(position.x + 20 - textWidth/2 - 2, position.y - 10, textWidth + 4, 14, Color{0, 0, 0, 150});
    DrawText(name, position.x + 20 - textWidth/2, position.y - 8, 12, WHITE);
    
    // Draw health bar with background (adjusted position and width)
    float healthPercent = (float)player.getHealth() / 100.0f;
    DrawRectangle(position.x - 2, position.y - 3, 44, 6, BLACK);
    DrawRectangle(position.x - 1, position.y - 2, 42, 4, MAROON);
    DrawRectangle(position.x - 1, position.y - 2, 42 * healthPercent, 4, LIME);
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
    // Load player and gun textures
    // Use relative path that works from build directory
    playerTexture_ = LoadTexture("../assets/player.png");
    gunTexture_ = LoadTexture("../assets/gun.png");
    
    if (playerTexture_.id == 0) {
        std::cerr << "Warning: Failed to load player.png from ../assets/" << std::endl;
        // Try loading from current directory as fallback
        playerTexture_ = LoadTexture("assets/player.png");
        if (playerTexture_.id == 0) {
            std::cerr << "Warning: Failed to load player.png from assets/" << std::endl;
        }
    }
    if (gunTexture_.id == 0) {
        std::cerr << "Warning: Failed to load gun.png from ../assets/" << std::endl;
        // Try loading from current directory as fallback
        gunTexture_ = LoadTexture("assets/gun.png");
        if (gunTexture_.id == 0) {
            std::cerr << "Warning: Failed to load gun.png from assets/" << std::endl;
        }
    }
}

void GameRenderer::unloadTextures() {
    // Unload textures
    if (playerTexture_.id != 0) UnloadTexture(playerTexture_);
    if (gunTexture_.id != 0) UnloadTexture(gunTexture_);
}