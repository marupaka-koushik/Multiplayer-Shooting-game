#include "Player.h"
#include <cmath>

Player::Player() 
    : id_(0), name_(""), x_(0), y_(0), velX_(0), velY_(0), 
      health_(100), maxHealth_(100), alive_(true), angle_(0), speed_(200.0f) {
}

Player::Player(int id, const std::string& name, float x, float y)
    : id_(id), name_(name), x_(x), y_(y), velX_(0), velY_(0),
      health_(100), maxHealth_(100), alive_(true), angle_(0), speed_(200.0f) {
}

void Player::setPosition(float x, float y) {
    x_ = x;
    y_ = y;
}

void Player::setVelocity(float velX, float velY) {
    velX_ = velX;
    velY_ = velY;
}

void Player::setHealth(int health) {
    health_ = health;
    if (health_ <= 0) {
        health_ = 0;
        alive_ = false;
    }
}

void Player::setAlive(bool alive) {
    alive_ = alive;
    if (alive && health_ <= 0) {
        health_ = maxHealth_;
    }
}

void Player::setAngle(float angle) {
    angle_ = angle;
}

void Player::update(float deltaTime) {
    if (!alive_) return;
    
    // Update position based on velocity
    x_ += velX_ * deltaTime;
    y_ += velY_ * deltaTime;
    
    // Apply basic physics (gravity, friction, etc.)
    // This is where you'd add game-specific movement logic
}

void Player::takeDamage(int damage) {
    if (!alive_) return;
    
    health_ -= damage;
    if (health_ <= 0) {
        health_ = 0;
        alive_ = false;
    }
}

void Player::respawn(float x, float y) {
    x_ = x;
    y_ = y;
    velX_ = 0;
    velY_ = 0;
    health_ = maxHealth_;
    alive_ = true;
}

bool Player::checkCollision(float x, float y, float width, float height) const {
    const float playerSize = 20.0f; // Player hitbox size
    
    return (x_ < x + width && 
            x_ + playerSize > x && 
            y_ < y + height && 
            y_ + playerSize > y);
}