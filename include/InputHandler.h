#pragma once
#include "raylib.h"

enum class InputAction {
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    SHOOT,
    AIM,
    NONE
};

struct InputState {
    bool moveLeft;
    bool moveRight;
    bool moveUp;
    bool moveDown;
    bool shoot;
    Vector2 mousePos;
    float aimAngle;
};

class InputHandler {
public:
    InputHandler();
    
    // Input processing
    void update();
    InputState getInputState() const { return currentState_; }
    
    // Check for specific actions
    bool isActionPressed(InputAction action) const;
    bool isActionDown(InputAction action) const;
    bool isActionReleased(InputAction action) const;
    
    // Mouse/aim handling
    Vector2 getMousePosition() const;
    float getAimAngle(Vector2 playerPos) const;
    
    // Configuration
    void setKeyBinding(InputAction action, KeyboardKey key);
    void setMouseSensitivity(float sensitivity);
    
private:
    InputState currentState_;
    InputState previousState_;
    float mouseSensitivity_;
    
    // Key bindings
    KeyboardKey moveLeftKey_;
    KeyboardKey moveRightKey_;
    KeyboardKey moveUpKey_;
    KeyboardKey moveDownKey_;
    
    void updateMovement();
    void updateAiming();
};