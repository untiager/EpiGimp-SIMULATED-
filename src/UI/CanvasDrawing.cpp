//Canvas drawing operations functionality
#include "../../include/UI/Canvas.hpp"
#include "../../include/Commands/DrawCommand.hpp"
#include "../../include/Core/HistoryManager.hpp"
#include <iostream>

namespace EpiGimp {

void Canvas::initializeDrawingTexture()
{
    if (currentTexture_) {
        const int width = (*currentTexture_)->width;
        const int height = (*currentTexture_)->height;
        drawingTexture_ = RenderTextureResource(width, height);
        
        // Clear the drawing texture to transparent
        drawingTexture_->clear(Color{0, 0, 0, 0});
        
        std::cout << "Drawing texture initialized: " << width << "x" << height << std::endl;
    }
}

void Canvas::drawStroke(Vector2 from, Vector2 to)
{
    if (!drawingVisible_ || !hasDrawingTexture()) return;
    
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
    
    std::cout << "Drawing stroke from (" << imageFrom.x << "," << imageFrom.y 
              << ") to (" << imageTo.x << "," << imageTo.y << ") on drawing layer" << std::endl;
    
    // Draw directly to the drawing texture
    drawingTexture_->beginDrawing();
    DrawLineEx(imageFrom, imageTo, 3.0f, drawingColor_); // Use selected drawing color
    drawingTexture_->endDrawing();
    
    std::cout << "Stroke drawn successfully" << std::endl;
}

void Canvas::handleDrawing()
{
    if (!hasImage()) return;
    
    const Vector2 mousePos = GetMousePosition();
    
    // Use image dimensions for rect calculation
    const Rectangle imageRect = calculateImageDestRect();
    
    // Check if mouse is over the image
    if (!CheckCollisionPointRec(mousePos, imageRect))
        return;
    
    static bool isDrawing = false;
    static Vector2 lastMousePos = {0, 0};
    static std::unique_ptr<DrawCommand> currentDrawCommand = nullptr;
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        std::cout << "Mouse pressed at (" << mousePos.x << "," << mousePos.y << "), tool=" << static_cast<int>(currentTool_) 
                  << ", over image=" << (CheckCollisionPointRec(mousePos, imageRect) ? "YES" : "NO") << std::endl;
        if (currentTool_ != DrawingTool::None) {
            isDrawing = true;
            lastMousePos = mousePos;
            
            // Create a new draw command if history manager is available
            if (historyManager_) {
                currentDrawCommand = createDrawCommand(this, "Draw stroke");
                std::cout << "Started drawing stroke, captured before state" << std::endl;
            }
        } else {
            std::cout << "Drawing tool is NONE - cannot draw" << std::endl;
        }
    }
    
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && isDrawing) {
        if (currentTool_ != DrawingTool::None) {
            std::cout << "Drawing from (" << lastMousePos.x << "," << lastMousePos.y << ") to (" << mousePos.x << "," << mousePos.y << ")" << std::endl;
            drawStroke(lastMousePos, mousePos);
            lastMousePos = mousePos;
        }
    }
    
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (isDrawing && currentDrawCommand && historyManager_) {
            std::cout << "Mouse released, capturing after state" << std::endl;
            // Capture the after state and execute the command
            currentDrawCommand->captureAfterState();
            
            if (historyManager_->executeCommand(std::move(currentDrawCommand))) {
                std::cout << "Drawing stroke completed and added to history" << std::endl;
            } else {
                std::cout << "Failed to add drawing stroke to history" << std::endl;
            }
            
            currentDrawCommand = nullptr;
        }
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