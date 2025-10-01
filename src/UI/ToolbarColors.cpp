//Color palette implementation for toolbar
#include "../../include/UI/Toolbar.hpp"
#include <iostream>
#include <algorithm>

namespace EpiGimp {

ColorPalette::ColorPalette(Rectangle bounds, EventDispatcher* dispatcher) 
    : bounds_(bounds), eventDispatcher_(dispatcher), selectedColor_(BLACK), selectedIndex_(0)
{
    if (!dispatcher)
        throw std::invalid_argument("EventDispatcher cannot be null");
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
        
        if (i == 0) // Select first color (black) by default
            swatch->isSelected = true;
        
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
    DrawRectangleRec(bounds_, LIGHTGRAY);
    DrawRectangleLinesEx(bounds_, 1, DARKGRAY);
    
    for (const auto& swatch : swatches_) {
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
    
    for (size_t i = 0; i < swatches_.size(); ++i) {
        swatches_[i]->isSelected = (ColorToInt(swatches_[i]->color) == ColorToInt(color));
        if (swatches_[i]->isSelected)
            selectedIndex_ = static_cast<int>(i);
    }
}

} // namespace EpiGimp