//Canvas drawing operations functionality
#include "../../include/UI/Canvas.hpp"
#include "../../include/Commands/DrawCommand.hpp"
#include "../../include/Core/HistoryManager.hpp"
#include <iostream>

namespace EpiGimp {

void Canvas::initializeDrawingTexture()
{
    if (selectedLayerIndex_ >= 0 && selectedLayerIndex_ < static_cast<int>(drawingLayers_.size()) && currentTexture_) {
        const int width = (*currentTexture_)->width;
        const int height = (*currentTexture_)->height;
        DrawingLayer& layer = drawingLayers_[selectedLayerIndex_];
        layer.texture = RenderTextureResource(width, height);
        
        layer.texture->clear(Color{0, 0, 0, 0});
        
        std::cout << "Drawing texture initialized for layer: " << layer.name << " (" << width << "x" << height << ")" << std::endl;
    }
}

void Canvas::drawStroke(Vector2 from, Vector2 to)
{
    if (!hasDrawingTexture()) return;
    
    const DrawingLayer& layer = drawingLayers_[selectedLayerIndex_];
    if (!layer.visible) return;
    
    const Rectangle imageRect = calculateImageDestRect();
    
    const Vector2 imageFrom = {
        (from.x - imageRect.x) / imageRect.width * (*currentTexture_)->width,
        (from.y - imageRect.y) / imageRect.height * (*currentTexture_)->height
    };
    
    const Vector2 imageTo = {
        (to.x - imageRect.x) / imageRect.width * (*currentTexture_)->width,
        (to.y - imageRect.y) / imageRect.height * (*currentTexture_)->height
    };
    
    std::cout << "Drawing stroke from (" << imageFrom.x << "," << imageFrom.y 
              << ") to (" << imageTo.x << "," << imageTo.y << ") on layer: " << layer.name << std::endl;
    
    layer.texture->beginDrawing();
    
    // Different tools have different characteristics
    switch (currentTool_) {
        case DrawingTool::Crayon:
            DrawLineEx(imageFrom, imageTo, 3.0f, drawingColor_);
            break;
            
        case DrawingTool::Brush:
        {
            // Brush tool: larger, softer strokes with multiple passes for smoother effect
            DrawLineEx(imageFrom, imageTo, 8.0f, drawingColor_);
            // Add some transparency for softer effect
            Color softerColor = drawingColor_;
            softerColor.a = 128; // Half transparency
            DrawLineEx(imageFrom, imageTo, 12.0f, softerColor);
            break;
        }
        
        case DrawingTool::None:
        default:
            // Fallback to basic line
            DrawLineEx(imageFrom, imageTo, 1.0f, drawingColor_);
            break;
    }
    
    layer.texture->endDrawing();
    
    std::cout << "Stroke drawn successfully with tool: " << static_cast<int>(currentTool_) << std::endl;
}

void Canvas::handleDrawing()
{
    if (!hasImage()) return;
    
    const Vector2 mousePos = GetMousePosition();
    
    const Rectangle imageRect = calculateImageDestRect();
    
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