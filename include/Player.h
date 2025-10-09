#pragma once
#include <string>

class Player {
public:
    Player();
    Player(int id, const std::string& name, float x = 0, float y = 0);
    
    // Getters
    int getId() const { return id_; }
    std::string getName() const { return name_; }
    float getX() const { return x_; }
    float getY() const { return y_; }
    float getVelX() const { return velX_; }
    float getVelY() const { return velY_; }
    int getHealth() const { return health_; }
    bool isAlive() const { return alive_; }
    float getAngle() const { return angle_; }
    
    // Setters
    void setPosition(float x, float y);
    void setVelocity(float velX, float velY);
    void setHealth(int health);
    void setAlive(bool alive);
    void setAngle(float angle);
    
    // Game logic
    void update(float deltaTime);
    void takeDamage(int damage);
    void respawn(float x, float y);
    bool checkCollision(float x, float y, float width, float height) const;
    
private:
    int id_;
    std::string name_;
    float x_, y_;
    float velX_, velY_;
    int health_;
    int maxHealth_;
    bool alive_;
    float angle_;
    float speed_;
};