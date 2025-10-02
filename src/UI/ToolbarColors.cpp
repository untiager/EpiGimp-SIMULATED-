//Color palette implementation for toolbar
#include "../../include/UI/Toolbar.hpp"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cstdio>

namespace EpiGimp {

ColorPalette::ColorPalette(Rectangle bounds, EventDispatcher* dispatcher) 
    : bounds_(bounds), eventDispatcher_(dispatcher), selectedColor_(BLACK), selectedIndex_(0),
      showRgbInput_(false)
{
    if (!dispatcher)
        throw std::invalid_argument("EventDispatcher cannot be null");
    
    strcpy(rgbInput_[0], "0");   // R
    strcpy(rgbInput_[1], "0");   // G  
    strcpy(rgbInput_[2], "0");   // B
    rgbInputActive_[0] = rgbInputActive_[1] = rgbInputActive_[2] = false;
    
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
    
    Rectangle rgbToggleButton = {bounds_.x + bounds_.width - 25, bounds_.y + bounds_.height - 20, 20, 15};
    if (CheckCollisionPointRec(mousePos, rgbToggleButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        toggleRgbInput();
    
    if (showRgbInput_) {
        updateRgbInput();
        return; // Don't process swatch clicks when RGB panel is open
    }
    
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
            
            // Update RGB input fields to reflect selected color
            if (showRgbInput_) {
                sprintf(rgbInput_[0], "%d", selectedColor_.r);
                sprintf(rgbInput_[1], "%d", selectedColor_.g);  
                sprintf(rgbInput_[2], "%d", selectedColor_.b);
            }
            
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
    
    Rectangle rgbToggleButton = {bounds_.x + bounds_.width - 25, bounds_.y + bounds_.height - 20, 20, 15};
    DrawRectangleRec(rgbToggleButton, showRgbInput_ ? BLUE : DARKGRAY);
    DrawRectangleLinesEx(rgbToggleButton, 1, BLACK);
    DrawText("RGB", static_cast<int>(rgbToggleButton.x + 1), static_cast<int>(rgbToggleButton.y + 2), 8, WHITE);
    
    if (showRgbInput_)
        drawRgbInput();
}

void ColorPalette::setSelectedColor(Color color)
{
    selectedColor_ = color;
    
    for (size_t i = 0; i < swatches_.size(); ++i) {
        swatches_[i]->isSelected = (ColorToInt(swatches_[i]->color) == ColorToInt(color));
        if (swatches_[i]->isSelected)
            selectedIndex_ = static_cast<int>(i);
    }
    
    if (showRgbInput_) {
        sprintf(rgbInput_[0], "%d", selectedColor_.r);
        sprintf(rgbInput_[1], "%d", selectedColor_.g);
        sprintf(rgbInput_[2], "%d", selectedColor_.b);
    }
}

void ColorPalette::toggleRgbInput()
{
    showRgbInput_ = !showRgbInput_;
    
    if (showRgbInput_) {
        // Initialize RGB input fields with current color
        sprintf(rgbInput_[0], "%d", selectedColor_.r);
        sprintf(rgbInput_[1], "%d", selectedColor_.g);
        sprintf(rgbInput_[2], "%d", selectedColor_.b);
        
        // Setup input field rectangles - make them larger and more spaced
        float inputWidth = 45;
        float inputHeight = 25;
        float startX = bounds_.x + 25;
        float startY = bounds_.y + bounds_.height - 50;
        
        rgbInputRects_[0] = {startX, startY, inputWidth, inputHeight};          // R
        rgbInputRects_[1] = {startX + 55, startY, inputWidth, inputHeight};     // G
        rgbInputRects_[2] = {startX + 110, startY, inputWidth, inputHeight};    // B
    }
}

void ColorPalette::updateRgbInput()
{
    Vector2 mousePos = GetMousePosition();
    
    for (int i = 0; i < 3; i++) {
        if (CheckCollisionPointRec(mousePos, rgbInputRects_[i]) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Deactivate all other fields
            for (int j = 0; j < 3; j++) {
                rgbInputActive_[j] = (j == i);
            }
        }
    }
    
    for (int i = 0; i < 3; i++) {
        if (rgbInputActive_[i]) {
            int key = GetCharPressed();
            
            // Handle numeric input
            if (key >= '0' && key <= '9') {
                int len = strlen(rgbInput_[i]);
                if (len < 3) { // Max 3 digits for RGB (0-255)
                    rgbInput_[i][len] = (char)key;
                    rgbInput_[i][len + 1] = '\0';
                }
            }
            
            // Handle backspace
            if (IsKeyPressed(KEY_BACKSPACE)) {
                int len = strlen(rgbInput_[i]);
                if (len > 0) {
                    rgbInput_[i][len - 1] = '\0';
                }
            }
            
            // Handle enter to apply color
            if (IsKeyPressed(KEY_ENTER)) {
                int r = atoi(rgbInput_[0]);
                int g = atoi(rgbInput_[1]);
                int b = atoi(rgbInput_[2]);
                
                // Clamp values to 0-255
                r = std::clamp(r, 0, 255);
                g = std::clamp(g, 0, 255);
                b = std::clamp(b, 0, 255);
                
                // Update the color
                selectedColor_ = Color{(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
                
                // Deselect all swatches since this is a custom color
                for (auto& swatch : swatches_) {
                    swatch->isSelected = false;
                }
                
                // Emit color changed event
                eventDispatcher_->emit<ColorChangedEvent>(ColorChangedEvent(selectedColor_));
                
                rgbInputActive_[i] = false;
            }
        }
    }
}

void ColorPalette::drawRgbInput() const
{
    Rectangle inputArea = {bounds_.x + 2, bounds_.y + bounds_.height - 55, bounds_.width - 4, 53};
    DrawRectangleRec(inputArea, Color{40, 40, 40, 255});
    DrawRectangleLinesEx(inputArea, 2, WHITE);
    
    const char* labels[] = {"R:", "G:", "B:"};
    
    for (int i = 0; i < 3; i++) {
        // Draw label - larger and better positioned
        DrawText(labels[i], static_cast<int>(rgbInputRects_[i].x - 18), static_cast<int>(rgbInputRects_[i].y + 7), 12, WHITE);
        
        // Draw input field with better styling
        Color fieldColor = rgbInputActive_[i] ? Color{120, 120, 255, 255} : Color{70, 70, 70, 255};
        DrawRectangleRec(rgbInputRects_[i], fieldColor);
        DrawRectangleLinesEx(rgbInputRects_[i], rgbInputActive_[i] ? 3 : 2, WHITE);
        
        // Draw input text - larger font and better centering
        DrawText(rgbInput_[i], static_cast<int>(rgbInputRects_[i].x + 5), static_cast<int>(rgbInputRects_[i].y + 7), 12, WHITE);
    }
    
    Rectangle colorPreview = {bounds_.x + bounds_.width - 35, bounds_.y + bounds_.height - 45, 30, 25};
    DrawRectangleRec(colorPreview, selectedColor_);
    DrawRectangleLinesEx(colorPreview, 2, WHITE);
    
    DrawText("Enter RGB values (0-255) and press ENTER", static_cast<int>(bounds_.x + 5), static_cast<int>(bounds_.y + bounds_.height - 20), 9, WHITE);
}

} // namespace EpiGimp