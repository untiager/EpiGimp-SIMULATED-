//Implementation of the toolbar UI components

#include "../../include/Class/ToolbarClass.hpp"
#include <iostream>

// Static member definitions
ToolbarClass::Button ToolbarClass::loadButton(Rectangle{0, 0, 0, 0}, "", nullptr);
ToolbarClass::Button ToolbarClass::saveButton(Rectangle{0, 0, 0, 0}, "", nullptr);
bool ToolbarClass::initialized = false;

void ToolbarClass::initialize(int screenWidth, std::function<void()> loadCallback, std::function<void()> saveCallback) {
    if (initialized) return;
    
    // Initialize Load button
    loadButton = Button(
        Rectangle{BUTTON_MARGIN, BUTTON_MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT},
        "Load Image",
        loadCallback
    );
    
    // Initialize Save button
    saveButton = Button(
        Rectangle{BUTTON_MARGIN * 2 + BUTTON_WIDTH, BUTTON_MARGIN, BUTTON_WIDTH, BUTTON_HEIGHT},
        "Save Image",
        saveCallback
    );
    
    initialized = true;
    std::cout << "Toolbar initialized successfully" << std::endl;
}

void ToolbarClass::update() {
    if (!initialized) return;
    
    updateButton(loadButton);
    updateButton(saveButton);
}

void ToolbarClass::draw() {
    if (!initialized) return;
    
    // Draw toolbar background
    DrawRectangle(0, 0, GetScreenWidth(), TOOLBAR_HEIGHT, RAYWHITE);
    DrawLine(0, TOOLBAR_HEIGHT, GetScreenWidth(), TOOLBAR_HEIGHT, LIGHTGRAY);
    
    // Draw buttons
    drawButton(loadButton);
    drawButton(saveButton);
}

void ToolbarClass::updateButton(Button& button) {
    Vector2 mousePos = GetMousePosition();
    
    // Check if mouse is over button
    button.isHovered = CheckCollisionPointRec(mousePos, button.rect);
    
    // Check if button is pressed
    if (button.isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        button.isPressed = true;
    }
    
    // Check if button is released (clicked)
    if (button.isPressed && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (button.isHovered && button.onClick) {
            button.onClick();
        }
        button.isPressed = false;
    }
    
    // Reset pressed state if mouse is not over button
    if (!button.isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        button.isPressed = false;
    }
}

void ToolbarClass::drawButton(const Button& button) {
    Color buttonColor = button.color;
    
    if (button.isPressed) {
        buttonColor = button.pressedColor;
    } else if (button.isHovered) {
        buttonColor = button.hoverColor;
    }
    
    // Draw button background
    DrawRectangleRec(button.rect, buttonColor);
    DrawRectangleLinesEx(button.rect, 1, DARKGRAY);
    
    // Draw button text
    int textWidth = MeasureText(button.text.c_str(), 16);
    int textX = button.rect.x + (button.rect.width - textWidth) / 2;
    int textY = button.rect.y + (button.rect.height - 16) / 2;
    
    DrawText(button.text.c_str(), textX, textY, 16, BLACK);
}