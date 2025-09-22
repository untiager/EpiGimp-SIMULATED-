//Toolbar button management and rendering
#include "../../include/UI/Toolbar.hpp"
#include <iostream>

namespace EpiGimp {

void Toolbar::updateButton(Button& button) const
{
    const Vector2 mousePos = GetMousePosition();
    
    button.isHovered = CheckCollisionPointRec(mousePos, button.bounds);
    
    if (button.isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        button.isPressed = true;
    
    if (button.isPressed && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (button.isHovered && button.onClick) {
            button.onClick();
        }
        button.isPressed = false;
    }
    
    if (!button.isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        button.isPressed = false;
}

void Toolbar::drawButton(const Button& button) const
{
    Color buttonColor = button.normalColor;
    
    if (button.isPressed) {
        buttonColor = button.pressedColor;
    } else if (button.isHovered) {
        buttonColor = button.hoverColor;
    }
    
    DrawRectangleRec(button.bounds, buttonColor);
    DrawRectangleLinesEx(button.bounds, 1, DARKGRAY);
    
    // Calculate text position for centering
    const int textWidth = MeasureText(button.text.c_str(), FONT_SIZE);
    const int textX = static_cast<int>(button.bounds.x + (button.bounds.width - textWidth) / 2);
    const int textY = static_cast<int>(button.bounds.y + (button.bounds.height - FONT_SIZE) / 2);
    
    DrawText(button.text.c_str(), textX, textY, FONT_SIZE, BLACK);
}

Rectangle Toolbar::calculateNextButtonBounds() const
{
    const float startX = bounds_.x + BUTTON_MARGIN;
    const float startY = bounds_.y + BUTTON_MARGIN;
    const float nextX = startX + static_cast<float>(buttons_.size()) * (BUTTON_WIDTH + BUTTON_MARGIN);
    
    return Rectangle{nextX, startY, BUTTON_WIDTH, BUTTON_HEIGHT};
}

} // namespace EpiGimp