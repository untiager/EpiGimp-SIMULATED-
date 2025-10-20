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
    
    // Convert screen coordinates to texture coordinates (0-1 range first)
    float normalizedFromX = (from.x - imageRect.x) / imageRect.width;
    float normalizedFromY = (from.y - imageRect.y) / imageRect.height;
    float normalizedToX = (to.x - imageRect.x) / imageRect.width;
    float normalizedToY = (to.y - imageRect.y) / imageRect.height;
    
    // Apply inverse flip transformations to match user expectation
    // When canvas is flipped visually, we need to "unflip" the input coordinates
    if (canvasFlippedHorizontal_) {
        normalizedFromX = 1.0f - normalizedFromX;
        normalizedToX = 1.0f - normalizedToX;
    }
    if (canvasFlippedVertical_) {
        normalizedFromY = 1.0f - normalizedFromY;
        normalizedToY = 1.0f - normalizedToY;
    }
    
    // Apply layer-specific flip transformations as well
    if (layer.flippedHorizontal) {
        normalizedFromX = 1.0f - normalizedFromX;
        normalizedToX = 1.0f - normalizedToX;
    }
    if (layer.flippedVertical) {
        normalizedFromY = 1.0f - normalizedFromY;
        normalizedToY = 1.0f - normalizedToY;
    }
    
    // Convert normalized coordinates to texture pixel coordinates
    const Vector2 imageFrom = {
        normalizedFromX * (*currentTexture_)->width,
        normalizedFromY * (*currentTexture_)->height
    };
    
    const Vector2 imageTo = {
        normalizedToX * (*currentTexture_)->width,
        normalizedToY * (*currentTexture_)->height
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
        
        case DrawingTool::Select:
            // Selection tool doesn't draw strokes, return early
            return;
        
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
    
    // Skip drawing logic for selection tool
    if (currentTool_ == DrawingTool::Select) return;
    
    const Vector2 mousePos = GetMousePosition();
    
    const Rectangle imageRect = calculateImageDestRect();
    
    if (!CheckCollisionPointRec(mousePos, imageRect))
        return;
    
    static bool isDrawingLeft = false;
    static bool isDrawingRight = false;
    static Vector2 lastMousePos = {0, 0};
    static std::unique_ptr<DrawCommand> currentDrawCommand = nullptr;
    static Color currentStrokeColor = BLACK;
    
    // Handle left mouse button (primary color)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        std::cout << "Left mouse pressed at (" << mousePos.x << "," << mousePos.y << "), tool=" << static_cast<int>(currentTool_) 
                  << ", primary color=RGB(" << static_cast<int>(primaryColor_.r) << "," 
                  << static_cast<int>(primaryColor_.g) << "," << static_cast<int>(primaryColor_.b) << ")" << std::endl;
        if (currentTool_ != DrawingTool::None) {
            isDrawingLeft = true;
            lastMousePos = mousePos;
            currentStrokeColor = primaryColor_;
            
            // Create a new draw command if history manager is available
            if (historyManager_) {
                currentDrawCommand = createDrawCommand(this, "Primary Color Stroke");
                std::cout << "Started primary color stroke, captured before state" << std::endl;
            }
        } else {
            std::cout << "Drawing tool is NONE - cannot draw" << std::endl;
        }
    }
    
    // Handle right mouse button (secondary color)
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        std::cout << "Right mouse pressed at (" << mousePos.x << "," << mousePos.y << "), tool=" << static_cast<int>(currentTool_) 
                  << ", secondary color=RGB(" << static_cast<int>(secondaryColor_.r) << "," 
                  << static_cast<int>(secondaryColor_.g) << "," << static_cast<int>(secondaryColor_.b) << ")" << std::endl;
        if (currentTool_ != DrawingTool::None) {
            isDrawingRight = true;
            lastMousePos = mousePos;
            currentStrokeColor = secondaryColor_;
            
            // Create a new draw command if history manager is available
            if (historyManager_) {
                currentDrawCommand = createDrawCommand(this, "Secondary Color Stroke");
                std::cout << "Started secondary color stroke, captured before state" << std::endl;
            }
        } else {
            std::cout << "Drawing tool is NONE - cannot draw" << std::endl;
        }
    }
    
    // Handle drawing while mouse is held down
    if ((IsMouseButtonDown(MOUSE_BUTTON_LEFT) && isDrawingLeft) || 
        (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && isDrawingRight)) {
        if (currentTool_ != DrawingTool::None) {
            std::cout << "Drawing from (" << lastMousePos.x << "," << lastMousePos.y << ") to (" << mousePos.x << "," << mousePos.y << ")" 
                      << " with color RGB(" << static_cast<int>(currentStrokeColor.r) << "," 
                      << static_cast<int>(currentStrokeColor.g) << "," << static_cast<int>(currentStrokeColor.b) << ")" << std::endl;
            // Temporarily set drawing color for the stroke
            Color originalColor = drawingColor_;
            drawingColor_ = currentStrokeColor;
            drawStroke(lastMousePos, mousePos);
            drawingColor_ = originalColor; // Restore original color
            lastMousePos = mousePos;
        }
    }
    
    // Handle mouse button release
    if ((IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && isDrawingLeft) || 
        (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && isDrawingRight)) {
        if ((isDrawingLeft || isDrawingRight) && currentDrawCommand && historyManager_) {
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
        isDrawingLeft = false;
        isDrawingRight = false;
    }
}

void Canvas::onColorChanged(const ColorChangedEvent& event)
{
    // Keep for backward compatibility - set as primary color
    primaryColor_ = event.selectedColor;
    drawingColor_ = event.selectedColor;
    std::cout << "Canvas: Drawing color changed to RGB(" 
              << static_cast<int>(drawingColor_.r) << ", "
              << static_cast<int>(drawingColor_.g) << ", "
              << static_cast<int>(drawingColor_.b) << ", "
              << static_cast<int>(drawingColor_.a) << ")" << std::endl;
}

void Canvas::onPrimaryColorChanged(const PrimaryColorChangedEvent& event)
{
    primaryColor_ = event.primaryColor;
    drawingColor_ = primaryColor_; // Update drawing color for compatibility
    std::cout << "Canvas: Primary color changed to RGB(" 
              << static_cast<int>(primaryColor_.r) << ", "
              << static_cast<int>(primaryColor_.g) << ", "
              << static_cast<int>(primaryColor_.b) << ", "
              << static_cast<int>(primaryColor_.a) << ")" << std::endl;
}

void Canvas::onSecondaryColorChanged(const SecondaryColorChangedEvent& event)
{
    secondaryColor_ = event.secondaryColor;
    std::cout << "Canvas: Secondary color changed to RGB(" 
              << static_cast<int>(secondaryColor_.r) << ", "
              << static_cast<int>(secondaryColor_.g) << ", "
              << static_cast<int>(secondaryColor_.b) << ", "
              << static_cast<int>(secondaryColor_.a) << ")" << std::endl;
}

} // namespace EpiGimp