//Toolbar implementation using modern C++
#ifndef TOOLBAR_HPP
#define TOOLBAR_HPP

#include <vector>
#include <memory>
#include <functional>
#include <string>
#include "raylib.h"
#include "../Core/Interfaces.hpp"
#include "../Core/EventSystem.hpp"

namespace EpiGimp {

class ColorPalette {
private:
    struct ColorSwatch {
        Rectangle bounds;
        Color color;
        bool isSelected = false;
        bool isHovered = false;
        
        ColorSwatch(Rectangle rect, Color c) : bounds(rect), color(c) {}
    };

    static constexpr int SWATCH_SIZE = 20;
    static constexpr int SWATCH_MARGIN = 2;
    static constexpr int PALETTE_PADDING = 5;

    Rectangle bounds_;
    std::vector<std::unique_ptr<ColorSwatch>> swatches_;
    EventDispatcher* eventDispatcher_;
    Color selectedColor_;
    int selectedIndex_;
    
    // RGB input state
    char rgbInput_[3][4];        // RGB input strings (R, G, B)
    bool rgbInputActive_[3];     // Which RGB input field is active
    Rectangle rgbInputRects_[3]; // Input field rectangles  
    mutable bool showRgbInput_;  // Whether to show the RGB input area
    
    // RGB Window properties
    mutable Rectangle rgbWindow_;
    mutable Rectangle rgbPreviewRect_;
    mutable Rectangle rgbCloseButton_;

    void initializePalette();

public:
    explicit ColorPalette(Rectangle bounds, EventDispatcher* dispatcher);
    
    void update(float deltaTime);
    void draw() const;
    Rectangle getBounds() const { return bounds_; }
    Color getSelectedColor() const { return selectedColor_; }
    void setSelectedColor(Color color);
    
    // RGB input methods
    void toggleRgbInput();
    void updateRgbInput();
    void drawRgbInput() const;
};

class Toolbar : public IToolbar {
private:
    struct Button {
        Rectangle bounds;
        std::string text;
        std::function<void()> onClick;
        Color normalColor;
        Color hoverColor;
        Color pressedColor;
        Color selectedColor;
        bool isHovered = false;
        bool isPressed = false;
        bool isSelected = false;
        
        Button(Rectangle rect, std::string buttonText, std::function<void()> callback)
            : bounds(rect), text(std::move(buttonText)), onClick(std::move(callback)),
              normalColor(LIGHTGRAY), hoverColor(GRAY), pressedColor(DARKGRAY), selectedColor(BLUE) {}
    };

    static constexpr int TOOLBAR_HEIGHT = 60;
    static constexpr int BUTTON_WIDTH = 120;
    static constexpr int BUTTON_HEIGHT = 40;
    static constexpr int BUTTON_MARGIN = 10;
    static constexpr int FONT_SIZE = 16;

    Rectangle bounds_;
    std::vector<std::unique_ptr<Button>> buttons_;
    EventDispatcher* eventDispatcher_;
    std::unique_ptr<ColorPalette> colorPalette_;
    DrawingTool currentTool_;
    
public:
    explicit Toolbar(Rectangle bounds, EventDispatcher* dispatcher);
    ~Toolbar() override = default;

    void update(float deltaTime) override;
    void draw() const override;
    Rectangle getBounds() const override { return bounds_; }

    // IToolbar interface
    void addButton(const std::string& text, std::function<void()> onClick) override;
    int getHeight() const override { return static_cast<int>(bounds_.height); }
    
    // Tool selection
    void setSelectedTool(DrawingTool tool);

private:
    void updateButton(Button& button) const;
    void drawButton(const Button& button) const;
    Rectangle calculateNextButtonBounds() const;
};

} // namespace EpiGimp

#endif // TOOLBAR_HPP