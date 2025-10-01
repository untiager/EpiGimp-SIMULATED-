#include "../../include/UI/LayerPanel.hpp"
#include <algorithm>
#include <iostream>

namespace EpiGimp {

LayerPanel::LayerPanel(Rectangle bounds, LayerManager* layerManager, EventDispatcher* dispatcher)
    : bounds_(bounds)
    , layerManager_(layerManager)
    , eventDispatcher_(dispatcher)
    , scrollOffset_(0)
    , selectedLayerIndex_(0)
    , isDragging_(false)
    , dragStartIndex_(-1)
    , dragOffset_{0, 0}
{
    
    if (!layerManager)
        throw std::invalid_argument("LayerManager cannot be null");
    
    if (!dispatcher)
        throw std::invalid_argument("EventDispatcher cannot be null");
    
    dispatcher->subscribe<LayerCreatedEvent>([this](const LayerCreatedEvent& event) {
        onLayerCreated(event);
    });
    
    dispatcher->subscribe<LayerDeletedEvent>([this](const LayerDeletedEvent& event) {
        onLayerDeleted(event);
    });
    
    dispatcher->subscribe<ActiveLayerChangedEvent>([this](const ActiveLayerChangedEvent& event) {
        onActiveLayerChanged(event);
    });
    
    dispatcher->subscribe<LayerVisibilityChangedEvent>([this](const LayerVisibilityChangedEvent& event) {
        onLayerVisibilityChanged(event);
    });
    
    dispatcher->subscribe<LayerReorderedEvent>([this](const LayerReorderedEvent& event) {
        onLayerReordered(event);
    });
    
    selectedLayerIndex_ = layerManager_->getActiveLayerIndex();
}

void LayerPanel::update(float /*deltaTime*/)
{
    handleInput();
    handleLayerDrag();
}

void LayerPanel::draw() const
{
    DrawRectangleRec(bounds_, LIGHTGRAY);
    DrawRectangleLinesEx(bounds_, 1, DARKGRAY);
    
    const char* title = "Layers";
    int titleWidth = MeasureText(title, 16);
    float titleX = bounds_.x + (bounds_.width - titleWidth) / 2;
    DrawText(title, static_cast<int>(titleX), static_cast<int>(bounds_.y + 5), 16, BLACK);
    
    BeginScissorMode(static_cast<int>(bounds_.x), 
                     static_cast<int>(bounds_.y + 25), 
                     static_cast<int>(bounds_.width), 
                     static_cast<int>(bounds_.height - 60));
    
    drawLayerList();
    
    EndScissorMode();
    
    drawLayerButtons();
}

void LayerPanel::refreshLayerList()
{
    selectedLayerIndex_ = layerManager_->getActiveLayerIndex();
    
    const int layerCount = static_cast<int>(layerManager_->getLayerCount());
    const int maxVisibleLayers = static_cast<int>((bounds_.height - 60) / LAYER_ITEM_HEIGHT);
    
    if (scrollOffset_ + maxVisibleLayers > layerCount)
        scrollOffset_ = std::max(0, layerCount - maxVisibleLayers);
}

void LayerPanel::scrollToLayer(size_t layerIndex)
{
    const int maxVisibleLayers = static_cast<int>((bounds_.height - 60) / LAYER_ITEM_HEIGHT);
    const int index = static_cast<int>(layerIndex);
    
    if (index < scrollOffset_) {
        scrollOffset_ = index;
    } else if (index >= scrollOffset_ + maxVisibleLayers) {
        scrollOffset_ = index - maxVisibleLayers + 1;
    }
    
    scrollOffset_ = std::max(0, scrollOffset_);
}

void LayerPanel::handleInput()
{
    if (!CheckCollisionPointRec(GetMousePosition(), bounds_))
        return;
    
    Vector2 mousePos = GetMousePosition();
    
    float wheelMove = GetMouseWheelMove();
    if (wheelMove != 0) {
        scrollOffset_ -= static_cast<int>(wheelMove * 2);
        const int layerCount = static_cast<int>(layerManager_->getLayerCount());
        const int maxVisibleLayers = static_cast<int>((bounds_.height - 60) / LAYER_ITEM_HEIGHT);
        scrollOffset_ = std::clamp(scrollOffset_, 0, std::max(0, layerCount - maxVisibleLayers));
    }
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        int layerIndex = getLayerIndexAtPosition(mousePos);
        if (layerIndex >= 0) {
            handleLayerItemClick(layerIndex, mousePos);
        } else {
            handleButtonClicks(mousePos);
        }
    }
}

void LayerPanel::handleLayerItemClick(int layerIndex, Vector2 mousePos)
{
    Rectangle itemRect = getLayerItemRect(layerIndex);
    Rectangle visibilityRect = getVisibilityButtonRect(itemRect);
    
    if (CheckCollisionPointRec(mousePos, visibilityRect)) {
        const Layer* layer = layerManager_->getLayer(layerIndex);
        if (layer)
            layerManager_->setLayerVisibility(layerIndex, !layer->isVisible());
    } else {
        // Select layer
        layerManager_->setActiveLayer(layerIndex);
        isDragging_ = true;
        dragStartIndex_ = layerIndex;
        dragOffset_ = {mousePos.x - itemRect.x, mousePos.y - itemRect.y};
    }
}

void LayerPanel::handleLayerDrag()
{
    if (isDragging_ && IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        int targetIndex = getLayerIndexAtPosition(mousePos);
        
        if (targetIndex >= 0 && targetIndex != dragStartIndex_) {
            // Visual feedback for drag operation
            // This would be implemented with more sophisticated UI feedback
        }
    }
    
    if (isDragging_ && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        Vector2 mousePos = GetMousePosition();
        int targetIndex = getLayerIndexAtPosition(mousePos);
        
        if (targetIndex >= 0 && targetIndex != dragStartIndex_)
            layerManager_->moveLayer(dragStartIndex_, targetIndex);
        
        isDragging_ = false;
        dragStartIndex_ = -1;
    }
}

void LayerPanel::handleButtonClicks(Vector2 mousePos)
{
    Rectangle buttonArea = {bounds_.x, bounds_.y + bounds_.height - 35, bounds_.width, 30};
    
    if (!CheckCollisionPointRec(mousePos, buttonArea))
        return;
    
    Rectangle newLayerBtn = {bounds_.x + 5, bounds_.y + bounds_.height - 30, 60, 25};
    if (CheckCollisionPointRec(mousePos, newLayerBtn)) {
        std::string layerName = "Layer " + std::to_string(layerManager_->getLayerCount() + 1);
        layerManager_->createLayer(layerName);
        return;
    }
    
    Rectangle deleteBtn = {bounds_.x + 70, bounds_.y + bounds_.height - 30, 60, 25};
    if (CheckCollisionPointRec(mousePos, deleteBtn) && layerManager_->getLayerCount() > 1) {
        layerManager_->deleteLayer(layerManager_->getActiveLayerIndex());
        return;
    }
    
    Rectangle duplicateBtn = {bounds_.x + 135, bounds_.y + bounds_.height - 30, 60, 25};
    if (CheckCollisionPointRec(mousePos, duplicateBtn)) {
        layerManager_->duplicateLayer(layerManager_->getActiveLayerIndex());
        return;
    }
}

void LayerPanel::drawLayerList() const
{
    const int layerCount = static_cast<int>(layerManager_->getLayerCount());
    
    for (int i = 0; i < layerCount; ++i) {
        if (!isLayerItemVisible(i)) continue;
        
        const Layer* layer = layerManager_->getLayer(i);
        if (!layer) continue;
        
        Rectangle itemRect = getLayerItemRect(i);
        bool isActive = (static_cast<size_t>(i) == layerManager_->getActiveLayerIndex());
        
        drawLayerItem(i, layer, itemRect, isActive);
    }
}

void LayerPanel::drawLayerItem(int index, const Layer* layer, Rectangle itemRect, bool isActive) const
{
    Color bgColor = isActive ? BLUE : (index % 2 == 0 ? WHITE : Color{245, 245, 245, 255});
    DrawRectangleRec(itemRect, bgColor);
    DrawRectangleLinesEx(itemRect, 1, GRAY);
    
    Rectangle visibilityRect = getVisibilityButtonRect(itemRect);
    drawVisibilityButton(visibilityRect, layer->isVisible());
    
    Rectangle nameRect = getLayerNameRect(itemRect);
    std::string layerName = layer->getName();
    if (layerName.length() > MAX_LAYER_NAME_LENGTH)
        layerName = layerName.substr(0, MAX_LAYER_NAME_LENGTH - 3) + "...";
    
    Color textColor = isActive ? WHITE : BLACK;
    DrawText(layerName.c_str(), 
             static_cast<int>(nameRect.x + 5), 
             static_cast<int>(nameRect.y + nameRect.height / 2 - 6), 
             12, textColor);
    
    if (layer->getOpacity() < 1.0f) {
        Rectangle opacityRect = {itemRect.x + itemRect.width - 30, itemRect.y + 5, 20, 8};
        DrawRectangleRec(opacityRect, DARKGRAY);
        float opacityWidth = opacityRect.width * layer->getOpacity();
        DrawRectangle(static_cast<int>(opacityRect.x), static_cast<int>(opacityRect.y), 
                     static_cast<int>(opacityWidth), static_cast<int>(opacityRect.height), GREEN);
    }
}

void LayerPanel::drawVisibilityButton(Rectangle rect, bool visible) const
{
    DrawRectangleRec(rect, WHITE);
    DrawRectangleLinesEx(rect, 1, BLACK);
    
    if (visible) {
        DrawText("V", static_cast<int>(rect.x + rect.width / 2 - 4), 
                static_cast<int>(rect.y + rect.height / 2 - 6), 12, BLACK);
    } else {
        DrawText("-", static_cast<int>(rect.x + rect.width / 2 - 3), 
                static_cast<int>(rect.y + rect.height / 2 - 6), 12, RED);
    }
}

void LayerPanel::drawLayerButtons() const
{
    Rectangle buttonArea = {bounds_.x, bounds_.y + bounds_.height - 35, bounds_.width, 30};
    DrawRectangleRec(buttonArea, GRAY);
    DrawRectangleLinesEx(buttonArea, 1, DARKGRAY);
    
    Rectangle newLayerBtn = {bounds_.x + 5, bounds_.y + bounds_.height - 30, 60, 25};
    DrawRectangleRec(newLayerBtn, LIGHTGRAY);
    DrawRectangleLinesEx(newLayerBtn, 1, BLACK);
    DrawText("New", static_cast<int>(newLayerBtn.x + 20), static_cast<int>(newLayerBtn.y + 8), 10, BLACK);
    
    Rectangle deleteBtn = {bounds_.x + 70, bounds_.y + bounds_.height - 30, 60, 25};
    Color deleteBtnColor = (layerManager_->getLayerCount() > 1) ? LIGHTGRAY : GRAY;
    DrawRectangleRec(deleteBtn, deleteBtnColor);
    DrawRectangleLinesEx(deleteBtn, 1, BLACK);
    DrawText("Del", static_cast<int>(deleteBtn.x + 22), static_cast<int>(deleteBtn.y + 8), 10, BLACK);
    
    Rectangle duplicateBtn = {bounds_.x + 135, bounds_.y + bounds_.height - 30, 60, 25};
    DrawRectangleRec(duplicateBtn, LIGHTGRAY);
    DrawRectangleLinesEx(duplicateBtn, 1, BLACK);
    DrawText("Dup", static_cast<int>(duplicateBtn.x + 20), static_cast<int>(duplicateBtn.y + 8), 10, BLACK);
}

Rectangle LayerPanel::getLayerItemRect(int index) const
{
    float y = bounds_.y + 25 + (index - scrollOffset_) * LAYER_ITEM_HEIGHT + LAYER_ITEM_PADDING;
    return Rectangle{bounds_.x + LAYER_ITEM_PADDING, y, 
                    bounds_.width - 2 * LAYER_ITEM_PADDING, LAYER_ITEM_HEIGHT - LAYER_ITEM_PADDING};
}

Rectangle LayerPanel::getVisibilityButtonRect(Rectangle itemRect) const
{
    return Rectangle{itemRect.x + 5, itemRect.y + (itemRect.height - VISIBILITY_BUTTON_SIZE) / 2, 
                    VISIBILITY_BUTTON_SIZE, VISIBILITY_BUTTON_SIZE};
}

Rectangle LayerPanel::getLayerNameRect(Rectangle itemRect) const
{
    return Rectangle{itemRect.x + 30, itemRect.y, itemRect.width - 60, itemRect.height};
}

bool LayerPanel::isLayerItemVisible(int index) const
{
    const int maxVisibleLayers = static_cast<int>((bounds_.height - 60) / LAYER_ITEM_HEIGHT);
    return index >= scrollOffset_ && index < scrollOffset_ + maxVisibleLayers;
}

int LayerPanel::getLayerIndexAtPosition(Vector2 pos) const
{
    if (pos.x < bounds_.x || pos.x > bounds_.x + bounds_.width ||
        pos.y < bounds_.y + 25 || pos.y > bounds_.y + bounds_.height - 35) {
        return -1;
    }
    
    int relativeY = static_cast<int>(pos.y - bounds_.y - 25);
    int index = scrollOffset_ + relativeY / static_cast<int>(LAYER_ITEM_HEIGHT);
    
    if (index >= 0 && index < static_cast<int>(layerManager_->getLayerCount()))
        return index;
    
    return -1;
}

void LayerPanel::onLayerCreated(const LayerCreatedEvent& /*event*/)
{
    refreshLayerList();
}

void LayerPanel::onLayerDeleted(const LayerDeletedEvent& /*event*/)
{
    refreshLayerList();
}

void LayerPanel::onActiveLayerChanged(const ActiveLayerChangedEvent& event)
{
    selectedLayerIndex_ = event.newIndex;
    scrollToLayer(event.newIndex);
}

void LayerPanel::onLayerVisibilityChanged(const LayerVisibilityChangedEvent& /*event*/)
{
    // No special handling needed, will be reflected in next draw
}

void LayerPanel::onLayerReordered(const LayerReorderedEvent& /*event*/)
{
    refreshLayerList();
}

} // namespace EpiGimp