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

class Toolbar : public IToolbar {
private:
    struct Button {
        Rectangle bounds;
        std::string text;
        std::function<void()> onClick;
        Color normalColor;
        Color hoverColor;
        Color pressedColor;
        bool isHovered = false;
        bool isPressed = false;
        
        Button(Rectangle rect, std::string buttonText, std::function<void()> callback)
            : bounds(rect), text(std::move(buttonText)), onClick(std::move(callback)),
              normalColor(LIGHTGRAY), hoverColor(GRAY), pressedColor(DARKGRAY) {}
    };

    static constexpr int TOOLBAR_HEIGHT = 60;
    static constexpr int BUTTON_WIDTH = 120;
    static constexpr int BUTTON_HEIGHT = 40;
    static constexpr int BUTTON_MARGIN = 10;
    static constexpr int FONT_SIZE = 16;

    Rectangle bounds_;
    std::vector<std::unique_ptr<Button>> buttons_;
    EventDispatcher* eventDispatcher_;
    
public:
    explicit Toolbar(Rectangle bounds, EventDispatcher* dispatcher);
    ~Toolbar() override = default;

    // IUIComponent interface
    void update(float deltaTime) override;
    void draw() const override;
    Rectangle getBounds() const override { return bounds_; }

    // IToolbar interface
    void addButton(const std::string& text, std::function<void()> onClick) override;
    int getHeight() const override { return static_cast<int>(bounds_.height); }

private:
    void updateButton(Button& button) const;
    void drawButton(const Button& button) const;
    Rectangle calculateNextButtonBounds() const;
};

} // namespace EpiGimp

#endif // TOOLBAR_HPP