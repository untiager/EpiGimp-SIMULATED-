#include "../../include/UI/SimpleLayerPanel.hpp"
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

SimpleLayerPanel::SimpleLayerPanel(Rectangle bounds, Canvas* canvas, EventDispatcher* dispatcher)
    : bounds_(bounds), canvas_(canvas), eventDispatcher_(dispatcher),
      backgroundHovered_(false), drawingHovered_(false)
{
    if (!canvas_) {
        throw std::invalid_argument("Canvas cannot be null");
    }
    if (!dispatcher) {
        throw std::invalid_argument("EventDispatcher cannot be null");
    }
    
    std::cout << "SimpleLayerPanel initialized with bounds: " 
              << bounds.x << ", " << bounds.y << ", " 
              << bounds.width << ", " << bounds.height << std::endl;
}

void SimpleLayerPanel::update(float /*deltaTime*/) {
    handleInput();
}

void SimpleLayerPanel::draw() const {
    // Panel background
    DrawRectangleRec(bounds_, Color{40, 40, 40, 255});
    DrawRectangleLinesEx(bounds_, 1, DARKGRAY);
    
    // Title
    DrawText("Layers", static_cast<int>(bounds_.x + 10), static_cast<int>(bounds_.y + 10), 16, WHITE);
    
    // Layer items
    Rectangle drawingRect = getDrawingLayerRect();
    Rectangle backgroundRect = getBackgroundLayerRect();
    
    // Drawing layer (on top in UI, rendered on top)
    drawLayerItem("Drawing", canvas_->isDrawingVisible(), drawingHovered_, drawingRect);
    
    // Background layer  
    drawLayerItem("Background", canvas_->isBackgroundVisible(), backgroundHovered_, backgroundRect);
    
    // Instructions
    DrawText("Click to toggle visibility", 
             static_cast<int>(bounds_.x + 10), 
             static_cast<int>(bounds_.y + bounds_.height - 25), 
             12, GRAY);
}

void SimpleLayerPanel::handleInput() {
    Vector2 mousePos = GetMousePosition();
    
    // Check hover states
    backgroundHovered_ = CheckCollisionPointRec(mousePos, getBackgroundLayerRect());
    drawingHovered_ = CheckCollisionPointRec(mousePos, getDrawingLayerRect());
    
    // Handle clicks
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (backgroundHovered_) {
            bool newState = !canvas_->isBackgroundVisible();
            canvas_->setBackgroundVisible(newState);
            std::cout << "Background layer " << (newState ? "shown" : "hidden") << std::endl;
        }
        if (drawingHovered_) {
            bool newState = !canvas_->isDrawingVisible();
            canvas_->setDrawingVisible(newState);
            std::cout << "Drawing layer " << (newState ? "shown" : "hidden") << std::endl;
        }
    }
}

void SimpleLayerPanel::drawLayerItem(const char* name, bool visible, bool& hovered, Rectangle itemRect) const {
    // Item background
    Color bgColor = hovered ? Color{60, 60, 60, 255} : Color{50, 50, 50, 255};
    DrawRectangleRec(itemRect, bgColor);
    DrawRectangleLinesEx(itemRect, 1, visible ? WHITE : GRAY);
    
    // Visibility indicator (eye icon)
    float eyeX = itemRect.x + 10;
    float eyeY = itemRect.y + itemRect.height / 2;
    if (visible) {
        DrawCircle(static_cast<int>(eyeX), static_cast<int>(eyeY), 6, WHITE);
        DrawCircle(static_cast<int>(eyeX), static_cast<int>(eyeY), 3, BLACK);
    } else {
        DrawCircle(static_cast<int>(eyeX), static_cast<int>(eyeY), 6, GRAY);
        DrawLine(static_cast<int>(eyeX - 8), static_cast<int>(eyeY - 8), 
                 static_cast<int>(eyeX + 8), static_cast<int>(eyeY + 8), RED);
    }
    
    // Layer name
    Color textColor = visible ? WHITE : GRAY;
    DrawText(name, 
             static_cast<int>(itemRect.x + 30), 
             static_cast<int>(itemRect.y + 10), 
             14, textColor);
}

Rectangle SimpleLayerPanel::getBackgroundLayerRect() const {
    return Rectangle{
        bounds_.x + 5,
        bounds_.y + 70,  // Below drawing layer
        bounds_.width - 10,
        30
    };
}

Rectangle SimpleLayerPanel::getDrawingLayerRect() const {
    return Rectangle{
        bounds_.x + 5,
        bounds_.y + 35,  // Above background layer
        bounds_.width - 10,
        30
    };
}

} // namespace EpiGimp