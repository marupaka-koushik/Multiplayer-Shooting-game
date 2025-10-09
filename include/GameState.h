#pragma once
#include "Player.h"
#include "Bullet.h"
#include <vector>
#include <map>
#include <string>

class GameState {
public:
    GameState();
    ~GameState();
    
    // Player management
    void addPlayer(int id, const std::string& name);
    void removePlayer(int id);
    Player* getPlayer(int id);
    const std::vector<Player*>& getAllPlayers() const { return players_; }
    
    // Bullet management
    void addBullet(int id, int ownerId, float x, float y, float angle, float speed);
    void removeBullet(int id);
    Bullet* getBullet(int id);
    const std::vector<Bullet*>& getAllBullets() const { return bullets_; }
    
    // Game logic
    void update(float deltaTime);
    void checkCollisions();
    void cleanupInactiveBullets();
    
    // Game settings
    float getWorldWidth() const { return worldWidth_; }
    float getWorldHeight() const { return worldHeight_; }
    void setWorldSize(float width, float height);
    
    // Serialization for networking
    std::string serialize() const;
    void deserialize(const std::string& data);
    
private:
    std::vector<Player*> players_;
    std::vector<Bullet*> bullets_;
    std::map<int, Player*> playerMap_;
    std::map<int, Bullet*> bulletMap_;
    
    float worldWidth_;
    float worldHeight_;
    int nextPlayerId_;
    int nextBulletId_;
    
    void checkPlayerBulletCollisions();
    void checkPlayerBoundaries();
};