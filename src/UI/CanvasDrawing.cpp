//Canvas drawing operations functionality
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

void Canvas::initializeDrawingLayer()
{
    if (hasImage() && currentTexture_) {
        const int width = (*currentTexture_)->width;
        const int height = (*currentTexture_)->height;
        drawingLayer_ = RenderTextureResource(width, height);
        
        // Clear the drawing layer to transparent
        drawingLayer_->clear(BLANK);
        
        std::cout << "Drawing layer initialized: " << width << "x" << height << std::endl;
    }
}

void Canvas::drawStroke(Vector2 from, Vector2 to)
{
    if (!drawingLayer_ || !drawingLayer_->isValid()) {
        return;
    }
    
    // Convert screen coordinates to image coordinates
    const Rectangle imageRect = calculateImageDestRect();
    
    // Transform screen coordinates to image texture coordinates
    const Vector2 imageFrom = {
        (from.x - imageRect.x) / imageRect.width * (*currentTexture_)->width,
        (from.y - imageRect.y) / imageRect.height * (*currentTexture_)->height
    };
    
    const Vector2 imageTo = {
        (to.x - imageRect.x) / imageRect.width * (*currentTexture_)->width,
        (to.y - imageRect.y) / imageRect.height * (*currentTexture_)->height
    };
    
    // Draw to the render texture
    drawingLayer_->beginDrawing();
    DrawLineEx(imageFrom, imageTo, 3.0f, drawingColor_); // Use selected drawing color
    drawingLayer_->endDrawing();
}

void Canvas::handleDrawing()
{
    if (!hasImage()) return;
    
    const Vector2 mousePos = GetMousePosition();
    const Rectangle imageRect = calculateImageDestRect();
    
    // Check if mouse is over the image
    if (!CheckCollisionPointRec(mousePos, imageRect)) {
        return;
    }
    
    static bool isDrawing = false;
    static Vector2 lastMousePos = {0, 0};
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (currentTool_ != DrawingTool::None) {
            isDrawing = true;
            lastMousePos = mousePos;
            // Only initialize drawing layer if it doesn't exist yet
            if (!drawingLayer_ || !drawingLayer_->isValid()) {
                initializeDrawingLayer();
            }
        }
    }
    
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && isDrawing) {
        if (currentTool_ != DrawingTool::None) {
            drawStroke(lastMousePos, mousePos);
            lastMousePos = mousePos;
        }
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        isDrawing = false;
    }
}

void Canvas::onColorChanged(const ColorChangedEvent& event)
{
    drawingColor_ = event.selectedColor;
    std::cout << "Canvas: Drawing color changed to RGB(" 
              << static_cast<int>(drawingColor_.r) << ", "
              << static_cast<int>(drawingColor_.g) << ", "
              << static_cast<int>(drawingColor_.b) << ", "
              << static_cast<int>(drawingColor_.a) << ")" << std::endl;
}

} // namespace EpiGimp