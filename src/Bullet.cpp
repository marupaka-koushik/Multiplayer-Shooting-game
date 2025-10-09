#include "Bullet.h"
#include <cmath>

Bullet::Bullet()
    : id_(0), ownerId_(0), x_(0), y_(0), velX_(0), velY_(0),
      active_(false), damage_(25), lifeTime_(0), maxLifeTime_(5.0f) {
}

Bullet::Bullet(int id, int ownerId, float x, float y, float angle, float speed)
    : id_(id), ownerId_(ownerId), x_(x), y_(y), active_(true),
      damage_(25), lifeTime_(0), maxLifeTime_(5.0f) {
    
    // Calculate velocity based on angle and speed
    velX_ = cos(angle) * speed;
    velY_ = sin(angle) * speed;
}

void Bullet::setPosition(float x, float y) {
    x_ = x;
    y_ = y;
}

void Bullet::setActive(bool active) {
    active_ = active;
}

void Bullet::update(float deltaTime) {
    if (!active_) return;
    
    // Update position
    x_ += velX_ * deltaTime;
    y_ += velY_ * deltaTime;
    
    // Update lifetime
    lifeTime_ += deltaTime;
    if (lifeTime_ >= maxLifeTime_) {
        active_ = false;
    }
}

bool Bullet::checkCollision(float x, float y, float width, float height) const {
    if (!active_) return false;
    
    const float bulletSize = 4.0f; // Bullet size
    
    return (x_ < x + width && 
            x_ + bulletSize > x && 
            y_ < y + height && 
            y_ + bulletSize > y);
}

bool Bullet::isOutOfBounds(float worldWidth, float worldHeight) const {
    return (x_ < 0 || x_ > worldWidth || y_ < 0 || y_ > worldHeight);
}