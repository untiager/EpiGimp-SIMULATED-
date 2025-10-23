//Canvas input handling functionality
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

void Canvas::handleInput()
{
    handleZoom();
    handlePanning();
    handleGlobalKeyboard();
    handleEyedropper();
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
    
    // V key to flip selection vertically
    if (IsKeyPressed(KEY_V)) {
        if (hasSelection_ && currentTool_ == DrawingTool::Select) {
            flipSelectionVertical();
        }
    }
    
    // H key to flip selection horizontally
    if (IsKeyPressed(KEY_H)) {
        if (hasSelection_ && currentTool_ == DrawingTool::Select) {
            flipSelectionHorizontal();
        }
    }
    
    // M key to toggle mirror mode
    if (IsKeyPressed(KEY_M)) {
        toggleMirrorMode();
        std::cout << "Mirror mode " << (mirrorModeEnabled_ ? "enabled" : "disabled") << std::endl;
    }
    
    // I key to select eyedropper tool
    if (IsKeyPressed(KEY_I)) {
        if (eventDispatcher_) {
            eventDispatcher_->emit<ToolSelectedEvent>(DrawingTool::Eyedropper);
            std::cout << "Eyedropper tool selected" << std::endl;
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
            // Get current image rect before zoom
            Rectangle oldImageRect = calculateImageDestRect();
            
            // Calculate mouse position relative to the current image center
            Vector2 imageCenterOld = {
                oldImageRect.x + oldImageRect.width / 2,
                oldImageRect.y + oldImageRect.height / 2
            };
            
            Vector2 mouseFromCenter = {
                mousePos.x - imageCenterOld.x,
                mousePos.y - imageCenterOld.y
            };
            
            // Apply zoom
            const float zoomFactor = wheel > 0 ? 1.2f : (1.0f / 1.2f);
            setZoom(zoomLevel_ * zoomFactor);
            
            // Get new image rect after zoom
            Rectangle newImageRect = calculateImageDestRect();
            Vector2 imageCenterNew = {
                newImageRect.x + newImageRect.width / 2,
                newImageRect.y + newImageRect.height / 2
            };
            
            // Calculate where the mouse should be relative to the new image center
            Vector2 targetMousePos = {
                imageCenterNew.x + mouseFromCenter.x * zoomFactor,
                imageCenterNew.y + mouseFromCenter.y * zoomFactor
            };
            
            // Adjust pan offset to keep mouse position fixed
            panOffset_.x += mousePos.x - targetMousePos.x;
            panOffset_.y += mousePos.y - targetMousePos.y;
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

void Canvas::handleEyedropper()
{
    if (!hasImage()) return;
    
    // Only handle input when eyedropper tool is active
    if (currentTool_ != DrawingTool::Eyedropper) return;
    
    const Vector2 mousePos = GetMousePosition();
    const Rectangle imageRect = calculateImageDestRect();
    
    // Check if mouse is over the image
    if (!CheckCollisionPointRec(mousePos, imageRect)) return;
    
    // Left click: Pick color and set as primary color
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Color pickedColor = pickColorAtScreenPosition(mousePos);
        
        // Emit event to update primary color
        if (eventDispatcher_) {
            eventDispatcher_->emit<PrimaryColorChangedEvent>(pickedColor);
            std::cout << "Eyedropper: Set primary color to RGB(" 
                      << static_cast<int>(pickedColor.r) << "," 
                      << static_cast<int>(pickedColor.g) << "," 
                      << static_cast<int>(pickedColor.b) << ")" << std::endl;
        }
    }
    
    // Right click: Pick color and set as secondary color
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        Color pickedColor = pickColorAtScreenPosition(mousePos);
        
        // Emit event to update secondary color
        if (eventDispatcher_) {
            eventDispatcher_->emit<SecondaryColorChangedEvent>(pickedColor);
            std::cout << "Eyedropper: Set secondary color to RGB(" 
                      << static_cast<int>(pickedColor.r) << "," 
                      << static_cast<int>(pickedColor.g) << "," 
                      << static_cast<int>(pickedColor.b) << ")" << std::endl;
        }
    }
}

} // namespace EpiGimp