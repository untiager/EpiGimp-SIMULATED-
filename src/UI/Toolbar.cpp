//Toolbar implementation
#include "../../include/UI/Toolbar.hpp"
#include <iostream>
#include <algorithm>

namespace EpiGimp {

Toolbar::Toolbar(Rectangle bounds, EventDispatcher* dispatcher) 
    : bounds_(bounds), eventDispatcher_(dispatcher)
{
    
    if (!dispatcher) {
        throw std::invalid_argument("EventDispatcher cannot be null");
    }
    
    std::cout << "Toolbar initialized with bounds: " 
              << bounds.x << ", " << bounds.y << ", " 
              << bounds.width << ", " << bounds.height << std::endl;
}

void Toolbar::update(float deltaTime)
{
    for (auto& button : buttons_) {
        updateButton(*button);
    }
}

void Toolbar::draw() const
{
    DrawRectangleRec(bounds_, RAYWHITE);
    DrawRectangleLinesEx(bounds_, 1, LIGHTGRAY);
    
    // Draw all buttons
    for (const auto& button : buttons_) {
        drawButton(*button);
    }
}

void Toolbar::addButton(const std::string& text, std::function<void()> onClick)
{
    const auto buttonBounds = calculateNextButtonBounds();
    auto button = std::make_unique<Button>(buttonBounds, text, std::move(onClick));
    buttons_.push_back(std::move(button));
    
    std::cout << "Added button: " << text << std::endl;
}

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