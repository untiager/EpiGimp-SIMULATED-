#include "../../include/UI/SimpleLayerPanel.hpp"
#include "../../include/UI/Canvas.hpp"
#include <iostream>
#include <algorithm>

namespace EpiGimp {

SimpleLayerPanel::SimpleLayerPanel(Rectangle bounds, Canvas* canvas, EventDispatcher* dispatcher)
    : bounds_(bounds), canvas_(canvas), eventDispatcher_(dispatcher),
      backgroundHovered_(false), addButtonHovered_(false), deleteButtonHovered_(false), 
      clearButtonHovered_(false), scrollOffset_(0.0f)
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
    drawLayerItem("Background", canvas_->isBackgroundVisible(), backgroundHovered_, false, backgroundRect);
    
    // Show all drawing layers
    int layerCount = canvas_->getLayerCount();
    int selectedLayer = canvas_->getSelectedLayerIndex();
    
    // Ensure we have enough hover states
    while (layerHovered_.size() < static_cast<size_t>(layerCount)) {
        layerHovered_.push_back(false);
    }
    
    for (int i = 0; i < layerCount; i++) {
        Rectangle layerRect = getLayerRect(i);
        bool isSelected = (i == selectedLayer);
        bool isVisible = canvas_->isLayerVisible(i);
        const std::string& layerName = canvas_->getLayerName(i);
        bool isHovered = i < static_cast<int>(layerHovered_.size()) ? layerHovered_[i] : false;
        
        drawLayerItem(layerName.c_str(), isVisible, isHovered, isSelected, layerRect);
    }
    
    // Layer management buttons
    Rectangle addButtonRect = getAddButtonRect();
    Rectangle deleteButtonRect = getDeleteButtonRect();
    Rectangle clearButtonRect = getClearButtonRect();
    
    bool hasSelectedLayer = selectedLayer >= 0;
    
    drawButton("Add", addButtonRect, addButtonHovered_, Color{0, 120, 0, 255});
    drawButton("Delete", deleteButtonRect, deleteButtonHovered_, 
               hasSelectedLayer ? Color{120, 0, 0, 255} : Color{60, 0, 0, 128});
    drawButton("Clear", clearButtonRect, clearButtonHovered_, 
               hasSelectedLayer ? Color{80, 80, 0, 255} : Color{40, 40, 0, 128});
    
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
    
    updateLayerHoverStates();
    
    // Handle clicks
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (backgroundHovered_) {
            bool newState = !canvas_->isBackgroundVisible();
            canvas_->setBackgroundVisible(newState);
            std::cout << "Background layer " << (newState ? "shown" : "hidden") << std::endl;
        }
        
        // Check layer clicks
        int layerCount = canvas_->getLayerCount();
        for (int i = 0; i < layerCount; i++) {
            if (i < static_cast<int>(layerHovered_.size()) && layerHovered_[i]) {
                Rectangle layerRect = getLayerRect(i);
                // Check if click is on the eye icon (visibility toggle)
                float eyeX = layerRect.x + 15;
                float eyeWidth = 20;
                if (mousePos.x >= eyeX && mousePos.x <= eyeX + eyeWidth) {
                    // Toggle visibility
                    bool newState = !canvas_->isLayerVisible(i);
                    canvas_->setLayerVisible(i, newState);
                } else {
                    // Select the layer
                    canvas_->setSelectedLayerIndex(i);
                    std::cout << "Selected layer: " << canvas_->getLayerName(i) << std::endl;
                }
                break;
            }
        }
        
        if (addButtonHovered_) {
            int newLayerIndex = canvas_->addNewDrawingLayer();
            if (newLayerIndex >= 0) {
                std::cout << "New drawing layer added (index " << newLayerIndex << ")" << std::endl;
            }
        }
        if (deleteButtonHovered_) {
            int selectedLayer = canvas_->getSelectedLayerIndex();
            if (selectedLayer >= 0) {
                canvas_->deleteLayer(selectedLayer);
            }
        }
        if (clearButtonHovered_) {
            int selectedLayer = canvas_->getSelectedLayerIndex();
            if (selectedLayer >= 0) {
                canvas_->clearLayer(selectedLayer);
            }
        }
    }
}

void SimpleLayerPanel::drawLayerItem(const char* name, bool visible, bool hovered, bool selected, Rectangle itemRect) const {
    // Item background - highlight if selected
    Color bgColor = selected ? Color{80, 120, 80, 255} : (hovered ? Color{60, 60, 60, 255} : Color{50, 50, 50, 255});
    DrawRectangleRec(itemRect, bgColor);
    DrawRectangleLinesEx(itemRect, 1, visible ? WHITE : GRAY);
    
    // Eye icon for visibility toggle
    float eyeX = itemRect.x + 15;
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
    if (selected) textColor = YELLOW; // Highlight selected layer text
    
    DrawText(name, 
             static_cast<int>(itemRect.x + 35), 
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

void SimpleLayerPanel::updateLayerHoverStates() {
    Vector2 mousePos = GetMousePosition();
    
    // Check background hover
    backgroundHovered_ = CheckCollisionPointRec(mousePos, getBackgroundLayerRect());
    
    // Check layer hovers
    int layerCount = canvas_->getLayerCount();
    while (layerHovered_.size() < static_cast<size_t>(layerCount)) {
        layerHovered_.push_back(false);
    }
    
    for (int i = 0; i < layerCount; i++) {
        if (i < static_cast<int>(layerHovered_.size())) {
            layerHovered_[i] = CheckCollisionPointRec(mousePos, getLayerRect(i));
        }
    }
    
    // Check button hovers
    int selectedLayer = canvas_->getSelectedLayerIndex();
    bool hasSelectedLayer = selectedLayer >= 0;
    
    addButtonHovered_ = CheckCollisionPointRec(mousePos, getAddButtonRect());
    deleteButtonHovered_ = hasSelectedLayer && CheckCollisionPointRec(mousePos, getDeleteButtonRect());
    clearButtonHovered_ = hasSelectedLayer && CheckCollisionPointRec(mousePos, getClearButtonRect());
}

Rectangle SimpleLayerPanel::getLayerRect(int layerIndex) const {
    return Rectangle{
        bounds_.x + 5,
        bounds_.y + 35 + (layerIndex * 35), // Stack layers vertically  
        bounds_.width - 10,
        30
    };
}

} // namespace EpiGimp