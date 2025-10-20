#ifndef SIMPLELAYERPANEL_HPP
#define SIMPLELAYERPANEL_HPP

#include "raylib.h"
#include "../Core/Interfaces.hpp"
#include "../Core/EventSystem.hpp"

namespace EpiGimp {

// Forward declarations
class Canvas;

/**
 * @brief Simple layer panel for the 2-layer system (background + drawing)
 */
class SimpleLayerPanel : public IUIComponent {
private:
    Rectangle bounds_;
    Canvas* canvas_;
    EventDispatcher* eventDispatcher_;
    
    mutable bool backgroundHovered_;
    mutable std::vector<bool> layerHovered_;  // Track hover state for each layer
    mutable bool addButtonHovered_;
    mutable bool deleteButtonHovered_;
    mutable bool clearButtonHovered_;
    mutable bool flipButtonHovered_;
    mutable bool flipHButtonHovered_;
    mutable float scrollOffset_;              // For scrolling through layers
    
    mutable bool isDragging_;                 // Whether a layer is being dragged
    mutable int dragStartIndex_;              // Index of the layer being dragged
    mutable Vector2 dragOffset_;              // Offset from mouse to layer item origin
    mutable Vector2 dragStartPos_;            // Mouse position when drag started
    
public:
    SimpleLayerPanel(Rectangle bounds, Canvas* canvas, EventDispatcher* dispatcher);
    ~SimpleLayerPanel() override = default;
    
    void update(float deltaTime) override;
    void draw() const override;
    Rectangle getBounds() const override { return bounds_; }
    
private:
    void handleInput();
    void handleLayerDrag();
    void drawLayerItem(const char* name, bool visible, bool hovered, bool selected, Rectangle itemRect, int layerIndex = -1) const;
    void drawButton(const char* text, Rectangle buttonRect, bool& hovered, Color baseColor) const;
    Rectangle getAddButtonRect() const;
    Rectangle getDeleteButtonRect() const;
    Rectangle getClearButtonRect() const;
    Rectangle getFlipButtonRect() const;
    Rectangle getFlipHButtonRect() const;
    void updateLayerHoverStates();
};

} // namespace EpiGimp

#endif // SIMPLELAYERPANEL_HPP