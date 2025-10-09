#include "InputHandler.h"
#include <cmath>

InputHandler::InputHandler() 
    : mouseSensitivity_(1.0f), moveLeftKey_(KEY_A), moveRightKey_(KEY_D), 
      moveUpKey_(KEY_W), moveDownKey_(KEY_S) {
    currentState_ = {};
    previousState_ = {};
}

void InputHandler::update() {
    // Store previous state
    previousState_ = currentState_;
    
    // Update current state
    updateMovement();
    updateAiming();
}

void InputHandler::updateMovement() {
    currentState_.moveLeft = IsKeyDown(moveLeftKey_);
    currentState_.moveRight = IsKeyDown(moveRightKey_);
    currentState_.moveUp = IsKeyDown(moveUpKey_);
    currentState_.moveDown = IsKeyDown(moveDownKey_);
    currentState_.shoot = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

void InputHandler::updateAiming() {
    currentState_.mousePos = GetMousePosition();
    // Aim angle will be calculated relative to player position when needed
}

bool InputHandler::isActionPressed(InputAction action) const {
    switch (action) {
        case InputAction::MOVE_LEFT:
            return currentState_.moveLeft && !previousState_.moveLeft;
        case InputAction::MOVE_RIGHT:
            return currentState_.moveRight && !previousState_.moveRight;
        case InputAction::MOVE_UP:
            return currentState_.moveUp && !previousState_.moveUp;
        case InputAction::MOVE_DOWN:
            return currentState_.moveDown && !previousState_.moveDown;
        case InputAction::SHOOT:
            return currentState_.shoot;
        default:
            return false;
    }
}

bool InputHandler::isActionDown(InputAction action) const {
    switch (action) {
        case InputAction::MOVE_LEFT:
            return currentState_.moveLeft;
        case InputAction::MOVE_RIGHT:
            return currentState_.moveRight;
        case InputAction::MOVE_UP:
            return currentState_.moveUp;
        case InputAction::MOVE_DOWN:
            return currentState_.moveDown;
        case InputAction::SHOOT:
            return IsMouseButtonDown(MOUSE_LEFT_BUTTON);
        default:
            return false;
    }
}

bool InputHandler::isActionReleased(InputAction action) const {
    switch (action) {
        case InputAction::MOVE_LEFT:
            return !currentState_.moveLeft && previousState_.moveLeft;
        case InputAction::MOVE_RIGHT:
            return !currentState_.moveRight && previousState_.moveRight;
        case InputAction::MOVE_UP:
            return !currentState_.moveUp && previousState_.moveUp;
        case InputAction::MOVE_DOWN:
            return !currentState_.moveDown && previousState_.moveDown;
        case InputAction::SHOOT:
            return IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
        default:
            return false;
    }
}

Vector2 InputHandler::getMousePosition() const {
    return currentState_.mousePos;
}

float InputHandler::getAimAngle(Vector2 playerPos) const {
    Vector2 mousePos = currentState_.mousePos;
    
    // Calculate angle from player to mouse
    float dx = mousePos.x - playerPos.x;
    float dy = mousePos.y - playerPos.y;
    
    return atan2(dy, dx);
}

void InputHandler::setKeyBinding(InputAction action, KeyboardKey key) {
    switch (action) {
        case InputAction::MOVE_LEFT:
            moveLeftKey_ = key;
            break;
        case InputAction::MOVE_RIGHT:
            moveRightKey_ = key;
            break;
        case InputAction::MOVE_UP:
            moveUpKey_ = key;
            break;
        case InputAction::MOVE_DOWN:
            moveDownKey_ = key;
            break;
        default:
            break;
    }
}

void InputHandler::setMouseSensitivity(float sensitivity) {
    mouseSensitivity_ = sensitivity;
}