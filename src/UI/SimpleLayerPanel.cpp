#include "../../include/UI/SimpleLayerPanel.hpp"
#include "../../include/UI/Canvas.hpp"
#include <iostream>
#include <algorithm>

namespace EpiGimp {

SimpleLayerPanel::SimpleLayerPanel(Rectangle bounds, Canvas* canvas, EventDispatcher* dispatcher)
    : bounds_(bounds), canvas_(canvas), eventDispatcher_(dispatcher),
      backgroundHovered_(false), drawingHovered_(false),
      addButtonHovered_(false), deleteButtonHovered_(false), clearButtonHovered_(false)
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
    Rectangle backgroundRect = getBackgroundLayerRect();
    
    // Always show background layer  
    drawLayerItem("Background", canvas_->isBackgroundVisible(), backgroundHovered_, backgroundRect);
    
    // Only show drawing layer if it actually exists
    if (canvas_->hasDrawingTexture()) {
        Rectangle drawingRect = getDrawingLayerRect();
        drawLayerItem("Drawing", canvas_->isDrawingVisible(), drawingHovered_, drawingRect);
    }
    
    // Layer management buttons
    Rectangle addButtonRect = getAddButtonRect();
    Rectangle deleteButtonRect = getDeleteButtonRect();
    Rectangle clearButtonRect = getClearButtonRect();
    
    bool hasDrawingLayer = canvas_->hasDrawingTexture();
    
    drawButton("Add", addButtonRect, addButtonHovered_, Color{0, 120, 0, 255});
    drawButton("Delete", deleteButtonRect, deleteButtonHovered_, 
               hasDrawingLayer ? Color{120, 0, 0, 255} : Color{60, 0, 0, 128});
    drawButton("Clear", clearButtonRect, clearButtonHovered_, 
               hasDrawingLayer ? Color{80, 80, 0, 255} : Color{40, 40, 0, 128});
    
    // Instructions
    DrawText("Click eye to toggle", 
             static_cast<int>(bounds_.x + 10), 
             static_cast<int>(bounds_.y + bounds_.height - 35), 
             12, GRAY);
    DrawText("Buttons manage layers", 
             static_cast<int>(bounds_.x + 10), 
             static_cast<int>(bounds_.y + bounds_.height - 20), 
             10, GRAY);
}

void SimpleLayerPanel::handleInput() {
    Vector2 mousePos = GetMousePosition();
    
    // Check hover states
    backgroundHovered_ = CheckCollisionPointRec(mousePos, getBackgroundLayerRect());
    
    // Only check drawing layer hover if it exists
    drawingHovered_ = canvas_->hasDrawingTexture() && CheckCollisionPointRec(mousePos, getDrawingLayerRect());
    
    bool hasDrawingLayer = canvas_->hasDrawingTexture();
    addButtonHovered_ = CheckCollisionPointRec(mousePos, getAddButtonRect());
    deleteButtonHovered_ = hasDrawingLayer && CheckCollisionPointRec(mousePos, getDeleteButtonRect());
    clearButtonHovered_ = hasDrawingLayer && CheckCollisionPointRec(mousePos, getClearButtonRect());
    
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
        if (addButtonHovered_) {
            canvas_->addNewDrawingLayer();
            std::cout << "New drawing layer added" << std::endl;
        }
        if (deleteButtonHovered_) {
            canvas_->deleteDrawingLayer();
            std::cout << "Drawing layer deleted" << std::endl;
        }
        if (clearButtonHovered_) {
            canvas_->clearDrawingLayer();
            std::cout << "Drawing layer cleared" << std::endl;
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

void SimpleLayerPanel::drawButton(const char* text, Rectangle buttonRect, bool& hovered, Color baseColor) const {
    Color bgColor = hovered ? Color{
        static_cast<unsigned char>(std::min(255, baseColor.r + 20)), 
        static_cast<unsigned char>(std::min(255, baseColor.g + 20)), 
        static_cast<unsigned char>(std::min(255, baseColor.b + 20)), 
        255
    } : baseColor;
    DrawRectangleRec(buttonRect, bgColor);
    DrawRectangleLinesEx(buttonRect, 1, WHITE);
    
    // Center text in button
    int textWidth = MeasureText(text, 12);
    float textX = buttonRect.x + (buttonRect.width - textWidth) / 2;
    float textY = buttonRect.y + (buttonRect.height - 12) / 2;
    
    DrawText(text, static_cast<int>(textX), static_cast<int>(textY), 12, WHITE);
}

Rectangle SimpleLayerPanel::getAddButtonRect() const {
    return Rectangle{
        bounds_.x + 5,
        bounds_.y + 110,
        55,
        25
    };
}

Rectangle SimpleLayerPanel::getDeleteButtonRect() const {
    return Rectangle{
        bounds_.x + 70,
        bounds_.y + 110,
        55,
        25
    };
}

Rectangle SimpleLayerPanel::getClearButtonRect() const {
    return Rectangle{
        bounds_.x + 135,
        bounds_.y + 110,
        55,
        25
    };
}

} // namespace EpiGimp