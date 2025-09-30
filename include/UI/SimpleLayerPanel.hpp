#ifndef LAYERPANEL_HPP
#define LAYERPANEL_HPP

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
    
    // UI state
    mutable bool backgroundHovered_;
    mutable bool drawingHovered_;
    
public:
    SimpleLayerPanel(Rectangle bounds, Canvas* canvas, EventDispatcher* dispatcher);
    ~SimpleLayerPanel() override = default;
    
    // IUIComponent interface
    void update(float deltaTime) override;
    void draw() const override;
    Rectangle getBounds() const override { return bounds_; }
    
private:
    void handleInput();
    void drawLayerItem(const char* name, bool visible, bool& hovered, Rectangle itemRect) const;
    Rectangle getBackgroundLayerRect() const;
    Rectangle getDrawingLayerRect() const;
};

} // namespace EpiGimp

#endif // LAYERPANEL_HPP