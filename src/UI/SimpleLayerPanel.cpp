#include "../../include/UI/SimpleLayerPanel.hpp"
#include "../../include/UI/Canvas.hpp"
#include <iostream>
#include <algorithm>

namespace EpiGimp {

SimpleLayerPanel::SimpleLayerPanel(Rectangle bounds, Canvas* canvas, EventDispatcher* dispatcher)
    : bounds_(bounds), canvas_(canvas), eventDispatcher_(dispatcher),
      backgroundHovered_(false), addButtonHovered_(false), deleteButtonHovered_(false), 
      clearButtonHovered_(false), flipButtonHovered_(false), flipHButtonHovered_(false), scrollOffset_(0.0f),
      isDragging_(false), dragStartIndex_(-1), dragOffset_{0, 0}, dragStartPos_{0, 0}
{
    if (!canvas_)
        throw std::invalid_argument("Canvas cannot be null");
    if (!dispatcher)
        throw std::invalid_argument("EventDispatcher cannot be null");
    
    std::cout << "SimpleLayerPanel initialized with bounds: " 
              << bounds.x << ", " << bounds.y << ", " 
              << bounds.width << ", " << bounds.height << std::endl;
}

void SimpleLayerPanel::update(float /*deltaTime*/)
{
    handleInput();
    handleLayerDrag();
}

void SimpleLayerPanel::draw() const
{
    DrawRectangleRec(bounds_, Color{40, 40, 40, 255});
    DrawRectangleLinesEx(bounds_, 1, DARKGRAY);
    
    DrawText("Layers", static_cast<int>(bounds_.x + 10), static_cast<int>(bounds_.y + 10), 16, WHITE);
    
    const float titleHeight = 25;
    const float layerHeight = 35;
    const float buttonHeight = 25;
    const float buttonMargin = 10;
    const float instructionHeight = 55;
    
    int layerCount = canvas_->getLayerCount();
    float totalLayersHeight = layerHeight * (layerCount + 1); // +1 for background
    float availableHeight = bounds_.height - titleHeight - buttonHeight - buttonMargin - instructionHeight;
    
    bool needsScrolling = totalLayersHeight > availableHeight;
    float maxScrollOffset = needsScrolling ? totalLayersHeight - availableHeight : 0;
    
    if (scrollOffset_ < 0) scrollOffset_ = 0;
    if (scrollOffset_ > maxScrollOffset) scrollOffset_ = maxScrollOffset;
    
    Rectangle layerArea = {
        bounds_.x,
        bounds_.y + titleHeight,
        bounds_.width,
        availableHeight
    };
    
    BeginScissorMode(static_cast<int>(layerArea.x), static_cast<int>(layerArea.y), 
                     static_cast<int>(layerArea.width), static_cast<int>(layerArea.height));
    
    float currentY = bounds_.y + titleHeight - scrollOffset_;
    
    Rectangle backgroundRect = {
        bounds_.x + 5,
        currentY + (layerCount * layerHeight), // Background goes after all drawing layers
        bounds_.width - 10,
        30
    };
    
    if (backgroundRect.y + backgroundRect.height > bounds_.y + titleHeight && 
        backgroundRect.y < bounds_.y + titleHeight + availableHeight) {
        drawLayerItem("Background", canvas_->isBackgroundVisible(), backgroundHovered_, false, backgroundRect);
    }
    
    int selectedLayer = canvas_->getSelectedLayerIndex();
    
    while (layerHovered_.size() < static_cast<size_t>(layerCount)) {
        layerHovered_.push_back(false);
    }
    
    for (int i = 0; i < layerCount; i++) {
        Rectangle layerRect = {
            bounds_.x + 5,
            currentY + (i * layerHeight),
            bounds_.width - 10,
            30
        };
        
        // Only draw if visible in the clipped area
        if (layerRect.y + layerRect.height > bounds_.y + titleHeight && 
            layerRect.y < bounds_.y + titleHeight + availableHeight) {
            
            bool isSelected = (i == selectedLayer);
            bool isVisible = canvas_->isLayerVisible(i);
            const std::string& layerName = canvas_->getLayerName(i);
            bool isHovered = i < static_cast<int>(layerHovered_.size()) ? layerHovered_[i] : false;
            
            drawLayerItem(layerName.c_str(), isVisible, isHovered, isSelected, layerRect, i);
        }
    }
    
    EndScissorMode();
    
    if (needsScrolling) {
        float scrollbarHeight = availableHeight * (availableHeight / totalLayersHeight);
        float scrollbarY = layerArea.y + (scrollOffset_ / maxScrollOffset) * (availableHeight - scrollbarHeight);
        
        Rectangle scrollbar = {
            bounds_.x + bounds_.width - 8,
            scrollbarY,
            6,
            scrollbarHeight
        };
        
        DrawRectangleRec(scrollbar, LIGHTGRAY);
    }
    
    Rectangle addButtonRect = getAddButtonRect();
    Rectangle deleteButtonRect = getDeleteButtonRect();
    Rectangle clearButtonRect = getClearButtonRect();
    Rectangle flipButtonRect = getFlipButtonRect();
    Rectangle flipHButtonRect = getFlipHButtonRect();
    
    bool hasSelectedLayer = selectedLayer >= 0;
    
    drawButton("Add", addButtonRect, addButtonHovered_, Color{0, 120, 0, 255});
    drawButton("Delete", deleteButtonRect, deleteButtonHovered_, 
               hasSelectedLayer ? Color{120, 0, 0, 255} : Color{60, 0, 0, 128});
    drawButton("Clear", clearButtonRect, clearButtonHovered_, 
               hasSelectedLayer ? Color{80, 80, 0, 255} : Color{40, 40, 0, 128});
    drawButton("Flip V", flipButtonRect, flipButtonHovered_, 
               hasSelectedLayer ? Color{0, 80, 120, 255} : Color{0, 40, 60, 128});
    drawButton("Flip H", flipHButtonRect, flipHButtonHovered_, 
               hasSelectedLayer ? Color{120, 0, 80, 255} : Color{60, 0, 40, 128});
    
    DrawText("Click eye to toggle", 
             static_cast<int>(bounds_.x + 10), 
             static_cast<int>(bounds_.y + bounds_.height - 35), 
             12, GRAY);
    DrawText("Scroll wheel: scroll layers", 
             static_cast<int>(bounds_.x + 10), 
             static_cast<int>(bounds_.y + bounds_.height - 20), 
             10, GRAY);
}

void SimpleLayerPanel::handleInput()
{
    Vector2 mousePos = GetMousePosition();
    
    if (CheckCollisionPointRec(mousePos, bounds_)) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            const float scrollSpeed = 30.0f;
            scrollOffset_ -= wheel * scrollSpeed;
        }
    }
    
    updateLayerHoverStates();
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (backgroundHovered_) {
            bool newState = !canvas_->isBackgroundVisible();
            canvas_->setBackgroundVisible(newState);
            std::cout << "Background layer " << (newState ? "shown" : "hidden") << std::endl;
        }
        
        int layerCount = canvas_->getLayerCount();
        for (int i = 0; i < layerCount; i++) {
            if (i < static_cast<int>(layerHovered_.size()) && layerHovered_[i]) {
                // Calculate actual layer rect with scroll offset
                const float titleHeight = 25;
                float currentY = bounds_.y + titleHeight - scrollOffset_;
                Rectangle layerRect = {
                    bounds_.x + 5,
                    currentY + (i * 35),
                    bounds_.width - 10,
                    30
                };
                
                if (CheckCollisionPointRec(mousePos, layerRect)) {
                    // Check if clicking on visibility toggle (eye icon area)
                    float eyeX = layerRect.x + 15;
                    float eyeWidth = 20;
                    if (mousePos.x >= eyeX && mousePos.x <= eyeX + eyeWidth) {
                        // Toggle visibility
                        bool newState = !canvas_->isLayerVisible(i);
                        canvas_->setLayerVisible(i, newState);
                        std::cout << "Layer " << i << " " << (newState ? "shown" : "hidden") << std::endl;
                    } else {
                        // Select the layer and start drag operation
                        canvas_->setSelectedLayerIndex(i);
                        std::cout << "Selected layer index: " << i << std::endl;
                        std::cout << "Selected layer: " << canvas_->getLayerName(i) << std::endl;
                        
                        // Start dragging
                        isDragging_ = true;
                        dragStartIndex_ = i;
                        dragStartPos_ = mousePos;
                        dragOffset_ = {mousePos.x - layerRect.x, mousePos.y - layerRect.y};
                    }
                    break;
                }
            }
        }
        
        if (addButtonHovered_) {
            int newLayerIndex = canvas_->addNewDrawingLayer();
            if (newLayerIndex >= 0)
                std::cout << "New drawing layer added (index " << newLayerIndex << ")" << std::endl;
        }
        if (deleteButtonHovered_) {
            int selectedLayer = canvas_->getSelectedLayerIndex();
            if (selectedLayer >= 0) {
                std::string layerName = canvas_->getLayerName(selectedLayer);
                canvas_->deleteLayer(selectedLayer);
                std::cout << "Deleted layer: " << layerName << std::endl;
            }
        }
        if (clearButtonHovered_) {
            int selectedLayer = canvas_->getSelectedLayerIndex();
            if (selectedLayer >= 0) {
                canvas_->clearLayer(selectedLayer);
                std::cout << "Cleared layer: " << canvas_->getLayerName(selectedLayer) << std::endl;
            }
        }
        if (flipButtonHovered_) {
            int selectedLayer = canvas_->getSelectedLayerIndex();
            if (selectedLayer >= 0) {
                canvas_->flipLayerVertical(selectedLayer);
                std::cout << "Flipped layer vertically: " << canvas_->getLayerName(selectedLayer) << std::endl;
            }
        }
        if (flipHButtonHovered_) {
            int selectedLayer = canvas_->getSelectedLayerIndex();
            if (selectedLayer >= 0) {
                canvas_->flipLayerHorizontal(selectedLayer);
                std::cout << "Flipped layer horizontally: " << canvas_->getLayerName(selectedLayer) << std::endl;
            }
        }
    }
}

void SimpleLayerPanel::drawLayerItem(const char* name, bool visible, bool hovered, bool selected, Rectangle itemRect, int layerIndex) const
{
    bool isBeingDragged = (isDragging_ && layerIndex == dragStartIndex_);
    
    Color bgColor = selected ? Color{80, 120, 80, 255} : (hovered ? Color{60, 60, 60, 255} : Color{50, 50, 50, 255});
    
    if (isBeingDragged)
        bgColor = Color{100, 100, 200, 200}; // Semi-transparent blue when dragging
    
    DrawRectangleRec(itemRect, bgColor);
    DrawRectangleLinesEx(itemRect, isBeingDragged ? 2 : 1, visible ? WHITE : GRAY);
    
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
    
    Color textColor = visible ? WHITE : GRAY;
    if (selected) textColor = YELLOW; // Highlight selected layer text
    
    DrawText(name, 
             static_cast<int>(itemRect.x + 35), 
             static_cast<int>(itemRect.y + 10), 
             14, textColor);
}

void SimpleLayerPanel::drawButton(const char* text, Rectangle buttonRect, bool& hovered, Color baseColor) const
{
    Color bgColor = hovered ? Color{
        static_cast<unsigned char>(std::min(255, baseColor.r + 20)), 
        static_cast<unsigned char>(std::min(255, baseColor.g + 20)), 
        static_cast<unsigned char>(std::min(255, baseColor.b + 20)), 
        255
    } : baseColor;
    DrawRectangleRec(buttonRect, bgColor);
    DrawRectangleLinesEx(buttonRect, 1, WHITE);
    
    int textWidth = MeasureText(text, 12);
    float textX = buttonRect.x + (buttonRect.width - textWidth) / 2;
    float textY = buttonRect.y + (buttonRect.height - 12) / 2;
    
    DrawText(text, static_cast<int>(textX), static_cast<int>(textY), 12, WHITE);
}

Rectangle SimpleLayerPanel::getAddButtonRect() const
{
    int layerCount = canvas_->getLayerCount();
    float buttonsY = bounds_.y + 35 + (layerCount * 35) + 40; // After layers + background + margin
    
    float maxY = bounds_.y + bounds_.height - 80; // Leave space for instructions
    if (buttonsY > maxY)
        buttonsY = maxY;
    
    return Rectangle{
        bounds_.x + 5,
        buttonsY,
        55,
        25
    };
}

Rectangle SimpleLayerPanel::getDeleteButtonRect() const
{
    int layerCount = canvas_->getLayerCount();
    float buttonsY = bounds_.y + 35 + (layerCount * 35) + 40; // After layers + background + margin
    
    float maxY = bounds_.y + bounds_.height - 80; // Leave space for instructions
    if (buttonsY > maxY)
        buttonsY = maxY;
    
    return Rectangle{
        bounds_.x + 70,
        buttonsY,
        55,
        25
    };
}

Rectangle SimpleLayerPanel::getClearButtonRect() const
{
    int layerCount = canvas_->getLayerCount();
    float buttonsY = bounds_.y + 35 + (layerCount * 35) + 40; // After layers + background + margin
    
    float maxY = bounds_.y + bounds_.height - 80; // Leave space for instructions
    if (buttonsY > maxY)
        buttonsY = maxY;
    
    return Rectangle{
        bounds_.x + 135,
        buttonsY,
        55,
        25
    };
}

Rectangle SimpleLayerPanel::getFlipButtonRect() const
{
    int layerCount = canvas_->getLayerCount();
    float buttonsY = bounds_.y + 35 + (layerCount * 35) + 40; // After layers + background + margin
    
    float maxY = bounds_.y + bounds_.height - 80; // Leave space for instructions
    if (buttonsY > maxY)
        buttonsY = maxY;
    
    return Rectangle{
        bounds_.x + 200,
        buttonsY,
        55,
        25
    };
}

Rectangle SimpleLayerPanel::getFlipHButtonRect() const
{
    int layerCount = canvas_->getLayerCount();
    float buttonsY = bounds_.y + 35 + (layerCount * 35) + 40; // After layers + background + margin
    
    float maxY = bounds_.y + bounds_.height - 80; // Leave space for instructions
    if (buttonsY > maxY)
        buttonsY = maxY;
    
    return Rectangle{
        bounds_.x + 265,
        buttonsY,
        55,
        25
    };
}

void SimpleLayerPanel::updateLayerHoverStates()
{
    Vector2 mousePos = GetMousePosition();
    
    const float titleHeight = 25;
    const float layerHeight = 35;
    const float buttonHeight = 25;
    const float buttonMargin = 10;
    const float instructionHeight = 55;
    
    int layerCount = canvas_->getLayerCount();
    float availableHeight = bounds_.height - titleHeight - buttonHeight - buttonMargin - instructionHeight;
    float currentY = bounds_.y + titleHeight - scrollOffset_;
    
    Rectangle backgroundRect = {
        bounds_.x + 5,
        currentY + (layerCount * layerHeight),
        bounds_.width - 10,
        30
    };
    
    Rectangle layerArea = {
        bounds_.x,
        bounds_.y + titleHeight,
        bounds_.width,
        availableHeight
    };
    
    backgroundHovered_ = CheckCollisionPointRec(mousePos, layerArea) && 
                        CheckCollisionPointRec(mousePos, backgroundRect) &&
                        backgroundRect.y + backgroundRect.height > bounds_.y + titleHeight && 
                        backgroundRect.y < bounds_.y + titleHeight + availableHeight;
    
    while (layerHovered_.size() < static_cast<size_t>(layerCount)) {
        layerHovered_.push_back(false);
    }
    
    for (int i = 0; i < layerCount; i++) {
        if (i < static_cast<int>(layerHovered_.size())) {
            Rectangle layerRect = {
                bounds_.x + 5,
                currentY + (i * layerHeight),
                bounds_.width - 10,
                30
            };
            
            // Only check hover if layer is visible in the scrollable area
            layerHovered_[i] = CheckCollisionPointRec(mousePos, layerArea) && 
                              CheckCollisionPointRec(mousePos, layerRect) &&
                              layerRect.y + layerRect.height > bounds_.y + titleHeight && 
                              layerRect.y < bounds_.y + titleHeight + availableHeight;
        }
    }
    
    int selectedLayer = canvas_->getSelectedLayerIndex();
    bool hasSelectedLayer = selectedLayer >= 0;
    
    addButtonHovered_ = CheckCollisionPointRec(mousePos, getAddButtonRect());
    deleteButtonHovered_ = hasSelectedLayer && CheckCollisionPointRec(mousePos, getDeleteButtonRect());
    clearButtonHovered_ = hasSelectedLayer && CheckCollisionPointRec(mousePos, getClearButtonRect());
    flipButtonHovered_ = hasSelectedLayer && CheckCollisionPointRec(mousePos, getFlipButtonRect());
    flipHButtonHovered_ = hasSelectedLayer && CheckCollisionPointRec(mousePos, getFlipHButtonRect());
}

void SimpleLayerPanel::handleLayerDrag()
{
    if (isDragging_ && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        // Continue dragging - could add visual feedback here
        // For now, just track that we're still dragging
    }
    
    if (isDragging_ && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        
        const float titleHeight = 25;
        const float layerHeight = 35;
        float currentY = bounds_.y + titleHeight - scrollOffset_;
        
        int layerCount = canvas_->getLayerCount();
        int targetIndex = -1;
        
        // Check if mouse is over any layer
        for (int i = 0; i < layerCount; i++) {
            Rectangle layerRect = {
                bounds_.x + 5,
                currentY + (i * layerHeight),
                bounds_.width - 10,
                30
            };
            
            if (CheckCollisionPointRec(mousePos, layerRect)) {
                targetIndex = i;
                break;
            }
        }
        
        if (targetIndex >= 0 && targetIndex != dragStartIndex_) {
            canvas_->moveLayer(dragStartIndex_, targetIndex);
            std::cout << "Moved layer from " << dragStartIndex_ << " to " << targetIndex << std::endl;
        }
        
        isDragging_ = false;
        dragStartIndex_ = -1;
    }
}

} // namespace EpiGimp