#include "GameState.h"
#include <sstream>
#include <algorithm>

GameState::GameState() 
    : worldWidth_(800), worldHeight_(600), nextPlayerId_(1), nextBulletId_(1) {
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
    
    // Create new player at random spawn position
    float spawnX = worldWidth_ * 0.1f + (rand() % (int)(worldWidth_ * 0.8f));
    float spawnY = worldHeight_ * 0.5f;
    
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
    // Update all players
    for (Player* player : players_) {
        player->update(deltaTime);
    }
    
    // Update all bullets
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
}

void GameState::checkPlayerBulletCollisions() {
    for (Bullet* bullet : bullets_) {
        if (!bullet->isActive()) continue;
        
        for (Player* player : players_) {
            if (!player->isAlive()) continue;
            if (player->getId() == bullet->getOwnerId()) continue; // Don't hit yourself
            
            if (bullet->checkCollision(player->getX(), player->getY(), 20, 20)) {
                // Hit detected
                player->takeDamage(bullet->getDamage());
                bullet->setActive(false);
                break;
            }
        }
    }
}

void GameState::checkPlayerBoundaries() {
    for (Player* player : players_) {
        float x = player->getX();
        float y = player->getY();
        
        // Keep players within world bounds
        if (x < 0) player->setPosition(0, y);
        if (x > worldWidth_ - 20) player->setPosition(worldWidth_ - 20, y);
        if (y < 0) player->setPosition(x, 0);
        if (y > worldHeight_ - 20) player->setPosition(x, worldHeight_ - 20);
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
                << ":" << bullet->getY();
        }
    }
    
    return oss.str();
}

void GameState::deserialize(const std::string& data) {
    // Implementation for deserializing game state from network
    // This would parse the serialized string and update the game state
    // For now, this is a placeholder - you'd implement the parsing logic here
}