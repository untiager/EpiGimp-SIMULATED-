//Canvas input handling functionality
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

void Canvas::handleInput()
{
    handleZoom();
    handlePanning();
    handleGlobalKeyboard();
}

void Canvas::handleGlobalKeyboard()
{
    // Handle global keyboard shortcuts that work regardless of current tool
    
    // Delete/Backspace for deleting selection
    if (IsKeyPressed(KEY_DELETE) || IsKeyPressed(KEY_BACKSPACE)) {
        if (hasSelection_) {
            deleteSelectionWithCommand();
        }
    }
    
    // Ctrl+A for select all (only if using selection tool)
    if (IsKeyPressed(KEY_A) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) {
        if (currentTool_ == DrawingTool::Select) {
            selectAll();
        }
    }
    
    // Ctrl+D or Escape for deselect
    if ((IsKeyPressed(KEY_D) && (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL))) || 
        IsKeyPressed(KEY_ESCAPE)) {
        if (hasSelection_) {
            clearSelection();
        }
    }
}

void Canvas::handleZoom()
{
    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        const Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, bounds_)) {
            const float newZoom = zoomLevel_ + wheel * ZOOM_STEP;
            setZoom(newZoom);
        }
    }
}

void Canvas::handlePanning()
{
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        const Vector2 mouseDelta = GetMouseDelta();
        panOffset_.x += mouseDelta.x;
        panOffset_.y += mouseDelta.y;
    }
    
    if (IsKeyDown(KEY_LEFT)) panOffset_.x += PAN_SPEED;
    if (IsKeyDown(KEY_RIGHT)) panOffset_.x -= PAN_SPEED;
    if (IsKeyDown(KEY_UP)) panOffset_.y += PAN_SPEED;
    if (IsKeyDown(KEY_DOWN)) panOffset_.y -= PAN_SPEED;
}

} // namespace EpiGimp