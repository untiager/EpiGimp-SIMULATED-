// Implementation of RaylibInputHandler class
#include "../../include/Utils/Implementations.hpp"
#include "raylib.h"

namespace EpiGimp {

void RaylibInputHandler::update() {
    // Raylib handles input automatically, so nothing to do here
}

bool RaylibInputHandler::isKeyPressed(int key) const {
    return IsKeyPressed(key);
}

bool RaylibInputHandler::isKeyDown(int key) const {
    return IsKeyDown(key);
}

bool RaylibInputHandler::isMouseButtonPressed(int button) const {
    return IsMouseButtonPressed(button);
}

bool RaylibInputHandler::isMouseButtonDown(int button) const {
    return IsMouseButtonDown(button);
}

Vector2 RaylibInputHandler::getMousePosition() const {
    return GetMousePosition();
}

Vector2 RaylibInputHandler::getMouseDelta() const {
    return GetMouseDelta();
}

float RaylibInputHandler::getMouseWheelMove() const {
    return GetMouseWheelMove();
}

} // namespace EpiGimp