#include "GameState.h"
#include <sstream>
#include <algorithm>

GameState::GameState() 
    : worldWidth_(2000), worldHeight_(1500), nextPlayerId_(1), nextBulletId_(1) {
    initializeObstacles();
}

GameState::~GameState() {
    // Clean up all players and bullets
    for (Player* player : players_) {
        delete player;
    }
    for (Bullet* bullet : bullets_) {
        delete bullet;
    }
}

void GameState::addPlayer(int id, const std::string& name) {
    // Check if player already exists
    if (playerMap_.find(id) != playerMap_.end()) {
        return;
    }
    
    // Find a valid spawn position
    float spawnX, spawnY;
    findValidSpawnPosition(spawnX, spawnY);
    
    Player* newPlayer = new Player(id, name, spawnX, spawnY);
    players_.push_back(newPlayer);
    playerMap_[id] = newPlayer;
}

void GameState::removePlayer(int id) {
    auto it = playerMap_.find(id);
    if (it != playerMap_.end()) {
        Player* player = it->second;
        
        // Remove from vector
        players_.erase(std::remove(players_.begin(), players_.end(), player), players_.end());
        
        // Remove from map
        playerMap_.erase(it);
        
        // Delete the player object
        delete player;
    }
}

Player* GameState::getPlayer(int id) {
    auto it = playerMap_.find(id);
    return (it != playerMap_.end()) ? it->second : nullptr;
}

void GameState::addBullet(int id, int ownerId, float x, float y, float angle, float speed) {
    // Check if bullet already exists
    if (bulletMap_.find(id) != bulletMap_.end()) {
        return;
    }
    
    Bullet* newBullet = new Bullet(id, ownerId, x, y, angle, speed);
    bullets_.push_back(newBullet);
    bulletMap_[id] = newBullet;
}

void GameState::removeBullet(int id) {
    auto it = bulletMap_.find(id);
    if (it != bulletMap_.end()) {
        Bullet* bullet = it->second;
        
        // Remove from vector
        bullets_.erase(std::remove(bullets_.begin(), bullets_.end(), bullet), bullets_.end());
        
        // Remove from map
        bulletMap_.erase(it);
        
        // Delete the bullet object
        delete bullet;
    }
}

Bullet* GameState::getBullet(int id) {
    auto it = bulletMap_.find(id);
    return (it != bulletMap_.end()) ? it->second : nullptr;
}

void GameState::update(float deltaTime) {
    // Apply movement to all players with collision checking
    for (Player* player : players_) {
        if (!player->isAlive()) continue;
        
        float velX = player->getVelX();
        float velY = player->getVelY();
        
        if (velX == 0 && velY == 0) continue; // No movement
        
        float currentX = player->getX();
        float currentY = player->getY();
        float newX = currentX + velX * deltaTime;
        float newY = currentY + velY * deltaTime;
        
        const float playerSize = 40.0f;
        
        // Check if new position would collide
        if (checkObstacleCollision(newX, newY, playerSize, playerSize)) {
            // Try sliding along X axis only
            if (!checkObstacleCollision(newX, currentY, playerSize, playerSize)) {
                player->setPosition(newX, currentY);
            }
            // Try sliding along Y axis only
            else if (!checkObstacleCollision(currentX, newY, playerSize, playerSize)) {
                player->setPosition(currentX, newY);
            }
            // Can't move, stay in place
            // Position unchanged
        } else {
            // Safe to move to new position
            player->setPosition(newX, newY);
        }
    }
    
    // Update all bullets (they move freely)
    for (Bullet* bullet : bullets_) {
        bullet->update(deltaTime);
    }
    
    // Check collisions
    checkCollisions();
    
    // Clean up inactive bullets
    cleanupInactiveBullets();
    
    // Check player boundaries
    checkPlayerBoundaries();
}

void GameState::checkCollisions() {
    checkPlayerBulletCollisions();
    checkBulletObstacleCollisions();
    checkPlayerObstacleCollisions();
}

void GameState::checkPlayerBulletCollisions() {
    for (Bullet* bullet : bullets_) {
        if (!bullet->isActive()) continue;
        
        for (Player* player : players_) {
            if (!player->isAlive()) continue;
            if (player->getId() == bullet->getOwnerId()) continue; // Don't hit yourself
            
            // Updated hitbox size to match new player size (40x40)
            if (bullet->checkCollision(player->getX(), player->getY(), 40, 40)) {
                // Hit detected
                bool wasAlive = player->isAlive();
                player->takeDamage(bullet->getDamage());
                bullet->setActive(false);
                
                // Award kill if player died from this hit
                if (wasAlive && !player->isAlive()) {
                    Player* shooter = getPlayer(bullet->getOwnerId());
                    if (shooter) {
                        shooter->addKill();
                    }
                }
                break;
            }
        }
    }
}

void GameState::checkPlayerBoundaries() {
    for (Player* player : players_) {
        float x = player->getX();
        float y = player->getY();
        float velX = player->getVelX();
        float velY = player->getVelY();
        bool positionChanged = false;
        
        // Keep players within world bounds (updated for 40x40 player size)
        if (x < 0) {
            x = 0;
            velX = 0;
            positionChanged = true;
        }
        if (x > worldWidth_ - 40) {
            x = worldWidth_ - 40;
            velX = 0;
            positionChanged = true;
        }
        if (y < 0) {
            y = 0;
            velY = 0;
            positionChanged = true;
        }
        if (y > worldHeight_ - 40) {
            y = worldHeight_ - 40;
            velY = 0;
            positionChanged = true;
        }
        
        if (positionChanged) {
            player->setPosition(x, y);
            player->setVelocity(velX, velY);
        }
    }
}

void GameState::cleanupInactiveBullets() {
    for (auto it = bullets_.begin(); it != bullets_.end();) {
        Bullet* bullet = *it;
        if (!bullet->isActive() || bullet->isOutOfBounds(worldWidth_, worldHeight_)) {
            // Remove from map
            bulletMap_.erase(bullet->getId());
            
            // Delete and remove from vector
            delete bullet;
            it = bullets_.erase(it);
        } else {
            ++it;
        }
    }
}

void GameState::setWorldSize(float width, float height) {
    worldWidth_ = width;
    worldHeight_ = height;
}

std::string GameState::serialize() const {
    std::ostringstream oss;
    
    // Serialize players
    oss << "PLAYERS:" << players_.size();
    for (const Player* player : players_) {
        oss << ":" << player->getId() 
            << ":" << player->getName()
            << ":" << player->getX() 
            << ":" << player->getY()
            << ":" << player->getHealth()
            << ":" << (player->isAlive() ? 1 : 0)
            << ":" << player->getAngle();
    }
    
    // Serialize bullets
    oss << "|BULLETS:" << bullets_.size();
    for (const Bullet* bullet : bullets_) {
        if (bullet->isActive()) {
            oss << ":" << bullet->getId()
                << ":" << bullet->getOwnerId()
                << ":" << bullet->getX()
                << ":" << bullet->getY()
                << ":" << bullet->getVelX()
                << ":" << bullet->getVelY();
        }
    }
    
    return oss.str();
}

void GameState::deserialize(const std::string& data) {
    // Parse the serialized game state
    // Format: "PLAYERS:count:id:name:x:y:health:alive:angle:...|BULLETS:count:id:ownerId:x:y:..."
    
    size_t pipePos = data.find('|');
    std::string playerData = data.substr(0, pipePos);
    std::string bulletData = (pipePos != std::string::npos) ? data.substr(pipePos + 1) : "";
    
    // Parse players
    std::istringstream playerStream(playerData);
    std::string token;
    
    // Skip "PLAYERS:" prefix and get count
    std::getline(playerStream, token, ':'); // "PLAYERS"
    std::getline(playerStream, token, ':'); // count
    int playerCount = std::stoi(token);
    
    // Clear existing players that aren't in the update
    std::map<int, bool> updatedPlayers;
    
    for (int i = 0; i < playerCount; i++) {
        std::getline(playerStream, token, ':'); // id
        int id = std::stoi(token);
        
        std::getline(playerStream, token, ':'); // name
        std::string name = token;
        
        std::getline(playerStream, token, ':'); // x
        float x = std::stof(token);
        
        std::getline(playerStream, token, ':'); // y
        float y = std::stof(token);
        
        std::getline(playerStream, token, ':'); // health
        int health = std::stoi(token);
        
        std::getline(playerStream, token, ':'); // alive
        bool alive = std::stoi(token) == 1;
        
        std::getline(playerStream, token, ':'); // angle
        float angle = std::stof(token);
        
        // Update or add player
        Player* player = getPlayer(id);
        if (player == nullptr) {
            addPlayer(id, name);
            player = getPlayer(id);
        }
        
        if (player) {
            player->setPosition(x, y);
            player->setHealth(health);
            player->setAlive(alive);
            player->setAngle(angle);
            updatedPlayers[id] = true;
        }
    }
    
    // Remove players that weren't in the update (disconnected players)
    for (auto it = players_.begin(); it != players_.end();) {
        Player* player = *it;
        if (updatedPlayers.find(player->getId()) == updatedPlayers.end()) {
            // Player not in update, remove them
            playerMap_.erase(player->getId());
            delete player;
            it = players_.erase(it);
        } else {
            ++it;
        }
    }
    
    // Parse bullets if present
    if (!bulletData.empty()) {
        std::istringstream bulletStream(bulletData);
        
        // Skip "BULLETS:" prefix and get count
        std::getline(bulletStream, token, ':'); // "BULLETS"
        std::getline(bulletStream, token, ':'); // count
        int bulletCount = std::stoi(token);
        
        // Clear existing bullets
        for (auto it = bullets_.begin(); it != bullets_.end();) {
            Bullet* bullet = *it;
            bulletMap_.erase(bullet->getId());
            delete bullet;
            it = bullets_.erase(it);
        }
        
        for (int i = 0; i < bulletCount; i++) {
            std::getline(bulletStream, token, ':'); // id
            int id = std::stoi(token);
            
            std::getline(bulletStream, token, ':'); // ownerId
            int ownerId = std::stoi(token);
            
            std::getline(bulletStream, token, ':'); // x
            float x = std::stof(token);
            
            std::getline(bulletStream, token, ':'); // y
            float y = std::stof(token);
            
            std::getline(bulletStream, token, ':'); // velX
            float velX = std::stof(token);
            
            std::getline(bulletStream, token, ':'); // velY
            float velY = std::stof(token);
            
            // Add bullet with placeholder values, then set correct velocity
            addBullet(id, ownerId, x, y, 0, 0);
            Bullet* bullet = getBullet(id);
            if (bullet) {
                bullet->setVelocity(velX, velY);
            }
        }
    }
}

void GameState::initializeObstacles() {
    // Initialize all obstacles matching the renderer's map layout EXACTLY
    obstacles_.clear();
    
    // === CENTRAL AREA ===
    // Central building/bunker (main structure, not door/windows)
    obstacles_.push_back(Obstacle(812, 1000, 375, 375));
    
    // Central vertical walls for cover
    obstacles_.push_back(Obstacle(950, 700, 100, 200));
    
    // === LEFT SIDE STRUCTURES ===
    // Left corner bunker
    obstacles_.push_back(Obstacle(50, 1150, 200, 225));
    
    // Left mid platforms
    obstacles_.push_back(Obstacle(125, 875, 300, 50));
    obstacles_.push_back(Obstacle(200, 925, 38, 450)); // Support pillar left
    obstacles_.push_back(Obstacle(312, 925, 38, 450)); // Support pillar right
    
    // Left upper platform
    obstacles_.push_back(Obstacle(375, 500, 250, 38));
    
    // Left side walls and obstacles
    obstacles_.push_back(Obstacle(250, 1100, 150, 50)); // Horizontal wall
    obstacles_.push_back(Obstacle(150, 600, 50, 200)); // Vertical wall
    obstacles_.push_back(Obstacle(450, 750, 50, 150)); // Vertical wall
    
    // === RIGHT SIDE STRUCTURES ===
    // Right corner bunker
    obstacles_.push_back(Obstacle(1750, 1150, 200, 225));
    
    // Right mid platforms
    obstacles_.push_back(Obstacle(1575, 875, 300, 50));
    obstacles_.push_back(Obstacle(1650, 925, 38, 450)); // Support pillar left
    obstacles_.push_back(Obstacle(1762, 925, 38, 450)); // Support pillar right
    
    // Right upper platform
    obstacles_.push_back(Obstacle(1375, 500, 250, 38));
    
    // Right side walls and obstacles
    obstacles_.push_back(Obstacle(1600, 1100, 150, 50)); // Horizontal wall
    obstacles_.push_back(Obstacle(1800, 600, 50, 200)); // Vertical wall
    obstacles_.push_back(Obstacle(1500, 750, 50, 150)); // Vertical wall
    
    // === TOP AREA ===
    // Top center platform
    obstacles_.push_back(Obstacle(875, 300, 250, 50));
    
    // Top left and right platforms
    obstacles_.push_back(Obstacle(200, 200, 200, 40));
    obstacles_.push_back(Obstacle(1600, 200, 200, 40));
    
    // Top floating obstacles
    obstacles_.push_back(Obstacle(600, 400, 80, 80));
    obstacles_.push_back(Obstacle(1320, 400, 80, 80));
    
    // === MIDDLE AREA OBSTACLES ===
    // Scattered cover boxes/crates
    // Left-center crates
    obstacles_.push_back(Obstacle(450, 1300, 100, 75));
    obstacles_.push_back(Obstacle(575, 1275, 88, 100));
    
    // Right-center crates
    obstacles_.push_back(Obstacle(1300, 1300, 100, 75));
    obstacles_.push_back(Obstacle(1425, 1275, 88, 100));
    
    // Mid-level scattered crates
    obstacles_.push_back(Obstacle(250, 1000, 75, 75));
    obstacles_.push_back(Obstacle(1200, 1300, 75, 75));
    obstacles_.push_back(Obstacle(750, 1300, 75, 75));
    
    // Small obstacles for tactical cover
    obstacles_.push_back(Obstacle(700, 950, 60, 60));
    obstacles_.push_back(Obstacle(1240, 950, 60, 60));
    obstacles_.push_back(Obstacle(500, 650, 60, 60));
    obstacles_.push_back(Obstacle(1440, 650, 60, 60));
    
    // === ADDITIONAL PLATFORMS ===
    // Lower mid platforms
    obstacles_.push_back(Obstacle(250, 1100, 200, 30));
    obstacles_.push_back(Obstacle(1550, 1100, 200, 30));
    
    // Diagonal cover walls
    obstacles_.push_back(Obstacle(350, 550, 40, 250));
    obstacles_.push_back(Obstacle(1610, 550, 40, 250));
    
    // Center-left and center-right vertical obstacles
    obstacles_.push_back(Obstacle(650, 800, 50, 150));
    obstacles_.push_back(Obstacle(1300, 800, 50, 150));
    
    // Small floating platforms for vertical gameplay
    obstacles_.push_back(Obstacle(100, 400, 100, 30));
    obstacles_.push_back(Obstacle(1800, 400, 100, 30));
    obstacles_.push_back(Obstacle(500, 250, 120, 30));
    obstacles_.push_back(Obstacle(1380, 250, 120, 30));
    
    // Top corners cover
    obstacles_.push_back(Obstacle(50, 50, 100, 100));
    obstacles_.push_back(Obstacle(1850, 50, 100, 100));
    
    // Bottom corners obstacles
    obstacles_.push_back(Obstacle(100, 1250, 80, 80));
    obstacles_.push_back(Obstacle(1820, 1250, 80, 80));
    
    // === MORE MID-LEVEL OBSTACLES ===
    // Horizontal cover walls at different heights
    obstacles_.push_back(Obstacle(800, 600, 150, 40));
    obstacles_.push_back(Obstacle(1050, 600, 150, 40));
    
    // Additional small platforms
    obstacles_.push_back(Obstacle(300, 750, 100, 25));
    obstacles_.push_back(Obstacle(1600, 750, 100, 25));
    
    // More tactical crates
    obstacles_.push_back(Obstacle(900, 1150, 70, 70));
    obstacles_.push_back(Obstacle(1030, 1150, 70, 70));
}

bool GameState::checkObstacleCollision(float x, float y, float width, float height) const {
    for (const Obstacle& obs : obstacles_) {
        // AABB collision detection
        if (x < obs.x + obs.width &&
            x + width > obs.x &&
            y < obs.y + obs.height &&
            y + height > obs.y) {
            return true;
        }
    }
    return false;
}

void GameState::checkBulletObstacleCollisions() {
    for (Bullet* bullet : bullets_) {
        if (!bullet->isActive()) continue;
        
        // Check if bullet hits any obstacle (bullets are small, use 5x5 hitbox)
        if (checkObstacleCollision(bullet->getX() - 2.5f, bullet->getY() - 2.5f, 5, 5)) {
            bullet->setActive(false);
        }
    }
}

void GameState::checkPlayerObstacleCollisions() {
    // This function now handles edge cases where players might be stuck in obstacles
    // (e.g., after respawn or network lag)
    for (Player* player : players_) {
        if (!player->isAlive()) continue;
        
        float currentX = player->getX();
        float currentY = player->getY();
        
        // If player somehow ended up inside an obstacle
        if (checkObstacleCollision(currentX, currentY, 40, 40)) {
            // Try to push player out in small increments
            const float pushStep = 2.0f;
            bool foundSafe = false;
            
            // Try pushing in 8 directions
            float directions[][2] = {
                {-1, 0}, {1, 0}, {0, -1}, {0, 1},
                {-1, -1}, {1, -1}, {-1, 1}, {1, 1}
            };
            
            // Try increasing push distances
            for (float multiplier = 1.0f; multiplier <= 10.0f && !foundSafe; multiplier += 1.0f) {
                for (int i = 0; i < 8; i++) {
                    float testX = currentX + directions[i][0] * pushStep * multiplier;
                    float testY = currentY + directions[i][1] * pushStep * multiplier;
                    
                    if (!checkObstacleCollision(testX, testY, 40, 40)) {
                        player->setPosition(testX, testY);
                        foundSafe = true;
                        break;
                    }
                }
            }
            
            // Always stop movement when stuck
            player->setVelocity(0, 0);
        }
    }
}

void GameState::respawnPlayer(int id) {
    Player* player = getPlayer(id);
    if (player && !player->isAlive()) {
        float spawnX, spawnY;
        findValidSpawnPosition(spawnX, spawnY);
        player->respawn(spawnX, spawnY);
    }
}

void GameState::findValidSpawnPosition(float& outX, float& outY) const {
    const float playerSize = 40.0f;
    int maxAttempts = 100;
    bool validPosition = false;
    
    for (int attempt = 0; attempt < maxAttempts; attempt++) {
        // Random position within the world bounds (with margins)
        outX = 50 + (rand() % (int)(worldWidth_ - 100));
        outY = 50 + (rand() % (int)(worldHeight_ - 100));
        
        // Check if this position collides with any obstacle
        if (!checkObstacleCollision(outX, outY, playerSize, playerSize)) {
            validPosition = true;
            break;
        }
    }
    
    // If we couldn't find a valid position after many attempts, use a safe default
    if (!validPosition) {
        outX = worldWidth_ * 0.5f;
        outY = 100;
    }
}