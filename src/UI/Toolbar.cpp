//Toolbar implementation
#include "../../include/UI/Toolbar.hpp"
#include <iostream>
#include <algorithm>

namespace EpiGimp {

// ColorPalette implementation
ColorPalette::ColorPalette(Rectangle bounds, EventDispatcher* dispatcher) 
    : bounds_(bounds), eventDispatcher_(dispatcher), selectedColor_(BLACK), selectedIndex_(0)
{
    if (!dispatcher) {
        throw std::invalid_argument("EventDispatcher cannot be null");
    }
    initializePalette();
}

void ColorPalette::initializePalette()
{
    // Define a set of commonly used colors for painting
    std::vector<Color> colors = {
        BLACK,              // 0 - Default selected
        WHITE,              // 1
        RED,                // 2
        GREEN,              // 3
        BLUE,               // 4
        YELLOW,             // 5
        ORANGE,             // 6
        PURPLE,             // 7
        BROWN,              // 8
        PINK,               // 9
        DARKGRAY,           // 10
        MAROON,             // 11
        DARKGREEN,          // 12
        DARKBLUE,           // 13
        Color{0, 82, 172, 255},  // 14 - Navy blue
        MAGENTA             // 15
    };
    
    // Calculate layout for color swatches
    const int swatchesPerRow = static_cast<int>((bounds_.width - 2 * PALETTE_PADDING) / (SWATCH_SIZE + SWATCH_MARGIN));
    const float startX = bounds_.x + PALETTE_PADDING;
    const float startY = bounds_.y + PALETTE_PADDING;
    
    swatches_.clear();
    for (size_t i = 0; i < colors.size(); ++i) {
        const int row = static_cast<int>(i) / swatchesPerRow;
        const int col = static_cast<int>(i) % swatchesPerRow;
        
        const float x = startX + col * (SWATCH_SIZE + SWATCH_MARGIN);
        const float y = startY + row * (SWATCH_SIZE + SWATCH_MARGIN);
        
        Rectangle swatchBounds = {x, y, SWATCH_SIZE, SWATCH_SIZE};
        auto swatch = std::make_unique<ColorSwatch>(swatchBounds, colors[i]);
        
        if (i == 0) { // Select first color (black) by default
            swatch->isSelected = true;
        }
        
        swatches_.push_back(std::move(swatch));
    }
}

void ColorPalette::update(float /*deltaTime*/)
{
    const Vector2 mousePos = GetMousePosition();
    
    for (size_t i = 0; i < swatches_.size(); ++i) {
        auto& swatch = *swatches_[i];
        
        // Check hover
        swatch.isHovered = CheckCollisionPointRec(mousePos, swatch.bounds);
        
        // Check click
        if (swatch.isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Deselect all swatches
            for (auto& otherSwatch : swatches_) {
                otherSwatch->isSelected = false;
            }
            
            // Select this swatch
            swatch.isSelected = true;
            selectedColor_ = swatch.color;
            selectedIndex_ = static_cast<int>(i);
            
            // Emit color changed event
            eventDispatcher_->emit<ColorChangedEvent>(ColorChangedEvent(selectedColor_));
        }
    }
}

void ColorPalette::draw() const
{
    // Draw palette background
    DrawRectangleRec(bounds_, LIGHTGRAY);
    DrawRectangleLinesEx(bounds_, 1, DARKGRAY);
    
    // Draw color swatches
    for (const auto& swatch : swatches_) {
        // Draw swatch background
        DrawRectangleRec(swatch->bounds, swatch->color);
        
        // Draw border - thicker if selected, different color if hovered
        Color borderColor = BLACK;
        float borderThickness = 1.0f;
        
        if (swatch->isSelected) {
            borderColor = WHITE;
            borderThickness = 3.0f;
        } else if (swatch->isHovered) {
            borderColor = GRAY;
            borderThickness = 2.0f;
        }
        
        DrawRectangleLinesEx(swatch->bounds, borderThickness, borderColor);
    }
}

void ColorPalette::setSelectedColor(Color color)
{
    selectedColor_ = color;
    
    // Find and select the matching swatch
    for (size_t i = 0; i < swatches_.size(); ++i) {
        swatches_[i]->isSelected = (ColorToInt(swatches_[i]->color) == ColorToInt(color));
        if (swatches_[i]->isSelected) {
            selectedIndex_ = static_cast<int>(i);
        }
    }
}

Toolbar::Toolbar(Rectangle bounds, EventDispatcher* dispatcher) 
    : bounds_(bounds), eventDispatcher_(dispatcher)
{
    
    if (!dispatcher) {
        throw std::invalid_argument("EventDispatcher cannot be null");
    }
    
    // Create color palette on the right side of the toolbar
    const float paletteWidth = 400; // Space for 16 colors in 2 rows
    const float paletteHeight = bounds.height - 10; // Leave some padding
    const float paletteX = bounds.x + bounds.width - paletteWidth - 10; // Right side with margin
    const float paletteY = bounds.y + 5; // Small top margin
    
    Rectangle paletteRect = {paletteX, paletteY, paletteWidth, paletteHeight};
    colorPalette_ = std::make_unique<ColorPalette>(paletteRect, dispatcher);
    
    std::cout << "Toolbar initialized with bounds: " 
              << bounds.x << ", " << bounds.y << ", " 
              << bounds.width << ", " << bounds.height << std::endl;
}

void Toolbar::update(float deltaTime)
{
    for (auto& button : buttons_) {
        updateButton(*button);
    }
    
    // Update color palette
    if (colorPalette_) {
        colorPalette_->update(deltaTime);
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
    
    // Draw color palette
    if (colorPalette_) {
        colorPalette_->draw();
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