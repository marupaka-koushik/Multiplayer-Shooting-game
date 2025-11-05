#pragma once

class Bullet {
public:
    Bullet();
    Bullet(int id, int ownerId, float x, float y, float angle, float speed);
    
    // Getters
    int getId() const { return id_; }
    int getOwnerId() const { return ownerId_; }
    float getX() const { return x_; }
    float getY() const { return y_; }
    float getVelX() const { return velX_; }
    float getVelY() const { return velY_; }
    bool isActive() const { return active_; }
    int getDamage() const { return damage_; }
    
    // Setters
    void setPosition(float x, float y);
    void setVelocity(float velX, float velY);
    void setActive(bool active);
    
    // Game logic
    void update(float deltaTime);
    bool checkCollision(float x, float y, float width, float height) const;
    bool isOutOfBounds(float worldWidth, float worldHeight) const;
    
private:
    int id_;
    int ownerId_;
    float x_, y_;
    float velX_, velY_;
    bool active_;
    int damage_;
    float lifeTime_;
    float maxLifeTime_;
};