#ifndef LAYERPANEL_HPP
#define LAYERPANEL_HPP

#include <memory>
#include <vector>
#include <string>
#include "raylib.h"
#include "../Core/Interfaces.hpp"
#include "../Core/LayerManager.hpp"
#include "../Core/EventSystem.hpp"

namespace EpiGimp {

/**
 * @brief UI panel for managing layers
 */
class LayerPanel : public IUIComponent {
private:
    Rectangle bounds_;
    LayerManager* layerManager_;
    EventDispatcher* eventDispatcher_;
    
    // UI state
    int scrollOffset_;
    int selectedLayerIndex_;
    bool isDragging_;
    int dragStartIndex_;
    Vector2 dragOffset_;
    
    // UI layout constants
    static constexpr float LAYER_ITEM_HEIGHT = 40.0f;
    static constexpr float LAYER_ITEM_PADDING = 4.0f;
    static constexpr float BUTTON_SIZE = 24.0f;
    static constexpr float VISIBILITY_BUTTON_SIZE = 20.0f;
    static constexpr int MAX_LAYER_NAME_LENGTH = 20;

public:
    explicit LayerPanel(Rectangle bounds, LayerManager* layerManager, EventDispatcher* dispatcher);
    ~LayerPanel() override = default;

    // Non-copyable but moveable
    LayerPanel(const LayerPanel&) = delete;
    LayerPanel& operator=(const LayerPanel&) = delete;
    LayerPanel(LayerPanel&&) = default;
    LayerPanel& operator=(LayerPanel&&) = default;

    // IUIComponent interface
    void update(float deltaTime) override;
    void draw() const override;
    Rectangle getBounds() const override { return bounds_; }

    // Layer panel specific
    void refreshLayerList();
    void scrollToLayer(size_t layerIndex);
    
private:
    void handleInput();
    void handleLayerItemClick(int layerIndex, Vector2 mousePos);
    void handleLayerDrag();
    void handleButtonClicks(Vector2 mousePos);
    
    void drawLayerList() const;
    void drawLayerItem(int index, const Layer* layer, Rectangle itemRect, bool isActive) const;
    void drawVisibilityButton(Rectangle rect, bool visible) const;
    void drawLayerButtons() const;
    
    Rectangle getLayerItemRect(int index) const;
    Rectangle getVisibilityButtonRect(Rectangle itemRect) const;
    Rectangle getLayerNameRect(Rectangle itemRect) const;
    
    bool isLayerItemVisible(int index) const;
    int getLayerIndexAtPosition(Vector2 pos) const;
    
    // Event handlers
    void onLayerCreated(const LayerCreatedEvent& event);
    void onLayerDeleted(const LayerDeletedEvent& event);
    void onActiveLayerChanged(const ActiveLayerChangedEvent& event);
    void onLayerVisibilityChanged(const LayerVisibilityChangedEvent& event);
    void onLayerReordered(const LayerReorderedEvent& event);
};

} // namespace EpiGimp

#endif // LAYERPANEL_HPP