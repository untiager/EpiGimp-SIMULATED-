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
    
    // T key to toggle transform mode
    if (IsKeyPressed(KEY_T)) {
        if (hasSelection_ && currentTool_ == DrawingTool::Select) {
            if (isTransformMode_) {
                exitTransformMode();
            } else {
                enterTransformMode();
            }
        }
    }
    
    // Zoom keyboard shortcuts
    if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
        // Ctrl+0: Fit to screen / Reset zoom
        if (IsKeyPressed(KEY_ZERO) || IsKeyPressed(KEY_KP_0)) {
            setZoom(1.0f);
            panOffset_ = {0, 0};
        }
        // Ctrl++: Zoom in
        else if (IsKeyPressed(KEY_KP_ADD) || IsKeyPressed(KEY_EQUAL)) {
            setZoom(zoomLevel_ * 1.2f);
        }
        // Ctrl+-: Zoom out  
        else if (IsKeyPressed(KEY_KP_SUBTRACT) || IsKeyPressed(KEY_MINUS)) {
            setZoom(zoomLevel_ / 1.2f);
        }
    }
}

void Canvas::handleZoom()
{
    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        const Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, bounds_)) {
            // Store mouse position relative to image before zoom
            Rectangle imageRect = calculateImageDestRect();
            Vector2 mouseInImage = {
                (mousePos.x - imageRect.x) / imageRect.width,
                (mousePos.y - imageRect.y) / imageRect.height
            };
            
            // Calculate new zoom with better scaling
            const float zoomFactor = wheel > 0 ? 1.2f : (1.0f / 1.2f);
            const float newZoom = zoomLevel_ * zoomFactor;
            
            // Apply zoom
            setZoom(newZoom);
            
            // Adjust pan to keep mouse position fixed
            if (mouseInImage.x >= 0 && mouseInImage.x <= 1 && 
                mouseInImage.y >= 0 && mouseInImage.y <= 1) {
                
                Rectangle newImageRect = calculateImageDestRect();
                Vector2 newMouseInImage = {
                    newImageRect.x + mouseInImage.x * newImageRect.width,
                    newImageRect.y + mouseInImage.y * newImageRect.height
                };
                
                // Adjust pan offset to keep mouse position consistent
                panOffset_.x += mousePos.x - newMouseInImage.x;
                panOffset_.y += mousePos.y - newMouseInImage.y;
            }
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