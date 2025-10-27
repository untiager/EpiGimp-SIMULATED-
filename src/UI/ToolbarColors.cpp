//Color palette implementation for toolbar
#include "../../include/UI/Toolbar.hpp"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <cstdio>

namespace EpiGimp {

ColorPalette::ColorPalette(Rectangle bounds, EventDispatcher* dispatcher) 
    : bounds_(bounds), eventDispatcher_(dispatcher), selectedColor_(BLACK), 
      primaryColor_(BLACK), secondaryColor_(WHITE), selectedIndex_(0), 
      primaryIndex_(0), secondaryIndex_(1), showRgbInput_(false)
{
    if (!dispatcher)
        throw std::invalid_argument("EventDispatcher cannot be null");
    
    strcpy(rgbInput_[0], "0");   // R
    strcpy(rgbInput_[1], "0");   // G  
    strcpy(rgbInput_[2], "0");   // B
    rgbInputActive_[0] = rgbInputActive_[1] = rgbInputActive_[2] = false;
    
    // Subscribe to color change events (e.g., from eyedropper)
    dispatcher->subscribe<PrimaryColorChangedEvent>([this](const PrimaryColorChangedEvent& event) {
        primaryColor_ = event.primaryColor;
        // Update RGB input if showing
        if (showRgbInput_) {
            sprintf(rgbInput_[0], "%d", primaryColor_.r);
            sprintf(rgbInput_[1], "%d", primaryColor_.g);  
            sprintf(rgbInput_[2], "%d", primaryColor_.b);
        }
        std::cout << "ColorPalette: Primary color updated to RGB(" 
                  << static_cast<int>(primaryColor_.r) << "," 
                  << static_cast<int>(primaryColor_.g) << "," 
                  << static_cast<int>(primaryColor_.b) << ")" << std::endl;
    });
    
    dispatcher->subscribe<SecondaryColorChangedEvent>([this](const SecondaryColorChangedEvent& event) {
        secondaryColor_ = event.secondaryColor;
        std::cout << "ColorPalette: Secondary color updated to RGB(" 
                  << static_cast<int>(secondaryColor_.r) << "," 
                  << static_cast<int>(secondaryColor_.g) << "," 
                  << static_cast<int>(secondaryColor_.b) << ")" << std::endl;
    });
    
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
    
    if (CheckCollisionPointRec(mousePos, rgbToggleButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        toggleRgbInput();
        return; // Don't process anything else this frame when toggling
    }
    
    if (showRgbInput_) {
        updateRgbInput();
        
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && 
            !CheckCollisionPointRec(mousePos, rgbWindow_) &&
            !CheckCollisionPointRec(mousePos, rgbToggleButton)) {
            showRgbInput_ = false;
        }
        
        return; // Don't process swatch clicks when RGB panel is open
    }
    
    for (size_t i = 0; i < swatches_.size(); ++i) {
        auto& swatch = *swatches_[i];
        
        // Check hover
        swatch.isHovered = CheckCollisionPointRec(mousePos, swatch.bounds);
        
        // Check left click (primary color)
        if (swatch.isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            primaryColor_ = swatch.color;
            primaryIndex_ = static_cast<int>(i);
            selectedColor_ = primaryColor_; // For backward compatibility
            selectedIndex_ = primaryIndex_;
            
            // Update RGB input fields to reflect primary color
            if (showRgbInput_) {
                sprintf(rgbInput_[0], "%d", primaryColor_.r);
                sprintf(rgbInput_[1], "%d", primaryColor_.g);  
                sprintf(rgbInput_[2], "%d", primaryColor_.b);
            }
            
            // Emit primary color changed event
            eventDispatcher_->emit<PrimaryColorChangedEvent>(PrimaryColorChangedEvent(primaryColor_));
            // Keep old event for backward compatibility
            eventDispatcher_->emit<ColorChangedEvent>(ColorChangedEvent(primaryColor_));
            
            std::cout << "Primary color selected: RGB(" << static_cast<int>(primaryColor_.r) 
                      << "," << static_cast<int>(primaryColor_.g) 
                      << "," << static_cast<int>(primaryColor_.b) << ")" << std::endl;
        }
        
        // Check right click (secondary color)
        if (swatch.isHovered && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            secondaryColor_ = swatch.color;
            secondaryIndex_ = static_cast<int>(i);
            
            // Emit secondary color changed event
            eventDispatcher_->emit<SecondaryColorChangedEvent>(SecondaryColorChangedEvent(secondaryColor_));
            
            std::cout << "Secondary color selected: RGB(" << static_cast<int>(secondaryColor_.r) 
                      << "," << static_cast<int>(secondaryColor_.g) 
                      << "," << static_cast<int>(secondaryColor_.b) << ")" << std::endl;
        }
    }
}

void ColorPalette::draw() const
{
    DrawRectangleRec(bounds_, LIGHTGRAY);
    DrawRectangleLinesEx(bounds_, 1, DARKGRAY);
    
    // Draw color swatches
    for (size_t i = 0; i < swatches_.size(); ++i) {
        const auto& swatch = swatches_[i];
        DrawRectangleRec(swatch->bounds, swatch->color);
        
        // Draw border - different styles for primary/secondary colors
        Color borderColor = BLACK;
        float borderThickness = 1.0f;
        
        if (static_cast<int>(i) == primaryIndex_) {
            // Primary color: thick blue border
            borderColor = BLUE;
            borderThickness = 3.0f;
        } else if (static_cast<int>(i) == secondaryIndex_) {
            // Secondary color: thick orange border
            borderColor = ORANGE;
            borderThickness = 3.0f;
        } else if (swatch->isHovered) {
            borderColor = GRAY;
            borderThickness = 2.0f;
        }
        
        DrawRectangleLinesEx(swatch->bounds, borderThickness, borderColor);
        
        // Add small corner indicators for primary/secondary
        if (static_cast<int>(i) == primaryIndex_) {
            // Primary indicator: small 'P' in top-left corner
            DrawText("P", static_cast<int>(swatch->bounds.x + 2), static_cast<int>(swatch->bounds.y + 1), 8, WHITE);
        }
        if (static_cast<int>(i) == secondaryIndex_) {
            // Secondary indicator: small 'S' in top-right corner
            DrawText("S", static_cast<int>(swatch->bounds.x + swatch->bounds.width - 10), static_cast<int>(swatch->bounds.y + 1), 8, WHITE);
        }
    }
    
    // Draw prominent primary/secondary color display
    // Position it to the far left to avoid overlapping with color swatches
    const float colorDisplaySize = 30.0f;
    const float colorDisplayX = bounds_.x - 150; // Move well to the left of the palette
    const float colorDisplayY = bounds_.y + bounds_.height - colorDisplaySize - 25;
    
    // Draw secondary color (behind, offset)
    Rectangle secondaryRect = {colorDisplayX + 8, colorDisplayY + 8, colorDisplaySize, colorDisplaySize};
    DrawRectangleRec(secondaryRect, secondaryColor_);
    DrawRectangleLinesEx(secondaryRect, 2.0f, ORANGE);
    
    // Draw primary color (in front)
    Rectangle primaryRect = {colorDisplayX, colorDisplayY, colorDisplaySize, colorDisplaySize};
    DrawRectangleRec(primaryRect, primaryColor_);
    DrawRectangleLinesEx(primaryRect, 2.0f, BLUE);
    
    // Add labels
    DrawText("L", static_cast<int>(primaryRect.x + 2), static_cast<int>(primaryRect.y + 2), 12, WHITE);
    DrawText("R", static_cast<int>(secondaryRect.x + secondaryRect.width - 12), static_cast<int>(secondaryRect.y + 2), 12, WHITE);
    
    // Draw instructions
    DrawText("L-Click: Primary", static_cast<int>(colorDisplayX + colorDisplaySize + 15), static_cast<int>(colorDisplayY), 8, BLACK);
    DrawText("R-Click: Secondary", static_cast<int>(colorDisplayX + colorDisplaySize + 15), static_cast<int>(colorDisplayY + 10), 8, BLACK);
    
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
    primaryColor_ = color; // Update primary color for backward compatibility
    
    for (size_t i = 0; i < swatches_.size(); ++i) {
        if (ColorToInt(swatches_[i]->color) == ColorToInt(color)) {
            primaryIndex_ = static_cast<int>(i);
            selectedIndex_ = primaryIndex_;
            break;
        }
    }
    
    if (showRgbInput_) {
        sprintf(rgbInput_[0], "%d", selectedColor_.r);
        sprintf(rgbInput_[1], "%d", selectedColor_.g);
        sprintf(rgbInput_[2], "%d", selectedColor_.b);
    }
}

void ColorPalette::setPrimaryColor(Color color)
{
    primaryColor_ = color;
    selectedColor_ = color; // Update selected color for backward compatibility
    
    for (size_t i = 0; i < swatches_.size(); ++i) {
        if (ColorToInt(swatches_[i]->color) == ColorToInt(color)) {
            primaryIndex_ = static_cast<int>(i);
            selectedIndex_ = primaryIndex_;
            break;
        }
    }
    
    if (showRgbInput_) {
        sprintf(rgbInput_[0], "%d", primaryColor_.r);
        sprintf(rgbInput_[1], "%d", primaryColor_.g);
        sprintf(rgbInput_[2], "%d", primaryColor_.b);
    }
}

void ColorPalette::setSecondaryColor(Color color)
{
    secondaryColor_ = color;
    
    for (size_t i = 0; i < swatches_.size(); ++i) {
        if (ColorToInt(swatches_[i]->color) == ColorToInt(color)) {
            secondaryIndex_ = static_cast<int>(i);
            break;
        }
    }
}

void ColorPalette::toggleRgbInput()
{
    showRgbInput_ = !showRgbInput_;
    
    if (showRgbInput_) {
        sprintf(rgbInput_[0], "%d", selectedColor_.r);
        sprintf(rgbInput_[1], "%d", selectedColor_.g);
        sprintf(rgbInput_[2], "%d", selectedColor_.b);
        
        float windowWidth = 350;
        float windowHeight = 200;
        float windowX = (GetScreenWidth() - windowWidth) / 2;
        float windowY = (GetScreenHeight() - windowHeight) / 2;
        
        rgbWindow_ = {windowX, windowY, windowWidth, windowHeight};
        
        float inputWidth = 60;
        float inputHeight = 30;
        float startX = windowX + 40;
        float startY = windowY + 60;
        
        rgbInputRects_[0] = {startX, startY, inputWidth, inputHeight};             // R
        rgbInputRects_[1] = {startX + 80, startY, inputWidth, inputHeight};        // G
        rgbInputRects_[2] = {startX + 160, startY, inputWidth, inputHeight};       // B
        
        rgbPreviewRect_ = {windowX + 250, windowY + 40, 80, 60};
        
        rgbCloseButton_ = {windowX + windowWidth - 35, windowY + 5, 25, 25};
    }
}

void ColorPalette::updateRgbInput()
{
    Vector2 mousePos = GetMousePosition();
    
    if (CheckCollisionPointRec(mousePos, rgbCloseButton_) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        showRgbInput_ = false;
        return;
    }
    
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
            bool inputChanged = false;
            
            if (key >= '0' && key <= '9') {
                int len = strlen(rgbInput_[i]);
                if (len < 3) { // Max 3 digits for RGB (0-255)
                    rgbInput_[i][len] = (char)key;
                    rgbInput_[i][len + 1] = '\0';
                    inputChanged = true;
                }
            }
            
            if (IsKeyPressed(KEY_BACKSPACE)) {
                int len = strlen(rgbInput_[i]);
                if (len > 0) {
                    rgbInput_[i][len - 1] = '\0';
                    inputChanged = true;
                }
            }
            
            // Update preview color in real-time when input changes
            if (inputChanged) {
                int r = strlen(rgbInput_[0]) > 0 ? atoi(rgbInput_[0]) : 0;
                int g = strlen(rgbInput_[1]) > 0 ? atoi(rgbInput_[1]) : 0;
                int b = strlen(rgbInput_[2]) > 0 ? atoi(rgbInput_[2]) : 0;
                
                // Clamp values to 0-255
                r = std::clamp(r, 0, 255);
                g = std::clamp(g, 0, 255);
                b = std::clamp(b, 0, 255);
                
                // Update preview color (but don't emit event yet)
                selectedColor_ = Color{(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                int r = strlen(rgbInput_[0]) > 0 ? atoi(rgbInput_[0]) : 0;
                int g = strlen(rgbInput_[1]) > 0 ? atoi(rgbInput_[1]) : 0;
                int b = strlen(rgbInput_[2]) > 0 ? atoi(rgbInput_[2]) : 0;
                
                // Clamp values to 0-255
                r = std::clamp(r, 0, 255);
                g = std::clamp(g, 0, 255);
                b = std::clamp(b, 0, 255);
                
                // Update the color and input fields to show the clamped values
                selectedColor_ = Color{(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
                primaryColor_ = selectedColor_; // Also update primary color
                sprintf(rgbInput_[0], "%d", r);
                sprintf(rgbInput_[1], "%d", g);
                sprintf(rgbInput_[2], "%d", b);
                
                // Reset palette selection indices since this is a custom color
                primaryIndex_ = -1;
                selectedIndex_ = -1;
                
                // Emit color changed events to apply the color
                eventDispatcher_->emit<PrimaryColorChangedEvent>(PrimaryColorChangedEvent(primaryColor_));
                eventDispatcher_->emit<ColorChangedEvent>(ColorChangedEvent(selectedColor_));
                
                rgbInputActive_[i] = false;
                showRgbInput_ = false; // Close the window after applying
            }
            
            if (IsKeyPressed(KEY_ESCAPE))
                showRgbInput_ = false;
        }
    }
}

void ColorPalette::drawRgbInput() const
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 100});
    
    DrawRectangleRec(rgbWindow_, Color{50, 50, 50, 255});
    DrawRectangleLinesEx(rgbWindow_, 3, WHITE);
    
    DrawText("RGB Color Picker", static_cast<int>(rgbWindow_.x + 20), static_cast<int>(rgbWindow_.y + 15), 16, WHITE);
    
    DrawRectangleRec(rgbCloseButton_, Color{200, 50, 50, 255});
    DrawRectangleLinesEx(rgbCloseButton_, 1, WHITE);
    DrawText("X", static_cast<int>(rgbCloseButton_.x + 8), static_cast<int>(rgbCloseButton_.y + 5), 14, WHITE);
    
    const char* labels[] = {"Red", "Green", "Blue"};
    Color labelColors[] = {Color{255, 100, 100, 255}, Color{100, 255, 100, 255}, Color{100, 100, 255, 255}};
    
    for (int i = 0; i < 3; i++) {
        // Draw colored label
        DrawText(labels[i], static_cast<int>(rgbInputRects_[i].x), static_cast<int>(rgbInputRects_[i].y - 25), 14, labelColors[i]);
        
        // Draw input field
        Color fieldColor = rgbInputActive_[i] ? Color{150, 150, 255, 255} : Color{80, 80, 80, 255};
        DrawRectangleRec(rgbInputRects_[i], fieldColor);
        DrawRectangleLinesEx(rgbInputRects_[i], rgbInputActive_[i] ? 3 : 2, WHITE);
        
        // Draw input text (centered)
        Vector2 textSize = MeasureTextEx(GetFontDefault(), rgbInput_[i], 16, 1);
        float textX = rgbInputRects_[i].x + (rgbInputRects_[i].width - textSize.x) / 2;
        float textY = rgbInputRects_[i].y + (rgbInputRects_[i].height - textSize.y) / 2;
        DrawText(rgbInput_[i], static_cast<int>(textX), static_cast<int>(textY), 16, WHITE);
        
        // Draw value label below input
        char valueText[10];
        sprintf(valueText, "(%d)", atoi(rgbInput_[i]));
        DrawText(valueText, static_cast<int>(rgbInputRects_[i].x + 15), static_cast<int>(rgbInputRects_[i].y + 35), 10, LIGHTGRAY);
    }
    
    DrawRectangleRec(rgbPreviewRect_, selectedColor_);
    DrawRectangleLinesEx(rgbPreviewRect_, 3, WHITE);
    DrawText("Preview", static_cast<int>(rgbPreviewRect_.x + 10), static_cast<int>(rgbPreviewRect_.y - 20), 12, WHITE);
    
    char rgbText[50];
    sprintf(rgbText, "RGB(%d, %d, %d)", selectedColor_.r, selectedColor_.g, selectedColor_.b);
    DrawText(rgbText, static_cast<int>(rgbPreviewRect_.x), static_cast<int>(rgbPreviewRect_.y + rgbPreviewRect_.height + 10), 12, WHITE);
    
    float instructionY = rgbWindow_.y + rgbWindow_.height - 60;
    DrawText("• Type RGB values (0-255) - preview updates in real-time", static_cast<int>(rgbWindow_.x + 20), static_cast<int>(instructionY), 10, LIGHTGRAY);
    DrawText("• Press ENTER to apply color and close", static_cast<int>(rgbWindow_.x + 20), static_cast<int>(instructionY + 15), 10, LIGHTGRAY);
    DrawText("• Press ESCAPE or click X to cancel", static_cast<int>(rgbWindow_.x + 20), static_cast<int>(instructionY + 30), 10, LIGHTGRAY);
}

} // namespace EpiGimp