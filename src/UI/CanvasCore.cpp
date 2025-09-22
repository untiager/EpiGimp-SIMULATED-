//Canvas core functionality
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

Canvas::Canvas(Rectangle bounds, EventDispatcher* dispatcher, HistoryManager* historyManager)
    : bounds_(bounds), zoomLevel_(1.0f), panOffset_{0, 0}, eventDispatcher_(dispatcher),
      historyManager_(historyManager), currentTool_(DrawingTool::None), isDrawing_(false), 
      lastMousePos_{0, 0}, drawingColor_(BLACK) // Initialize with black color
{
    
    if (!dispatcher)
        throw std::invalid_argument("EventDispatcher cannot be null");
    
    // Subscribe to color change events
    dispatcher->subscribe<ColorChangedEvent>([this](const ColorChangedEvent& event) {
        onColorChanged(event);
    });
    
    std::cout << "Canvas initialized with bounds: " 
              << bounds.x << ", " << bounds.y << ", " 
              << bounds.width << ", " << bounds.height << std::endl;
}

void Canvas::update(float /*deltaTime*/)
{
    handleInput();
    handleDrawing();
}

void Canvas::draw() const
{
    DrawRectangleRec(bounds_, WHITE);
    DrawRectangleLinesEx(bounds_, 1, DARKGRAY);
    
    BeginScissorMode(static_cast<int>(bounds_.x), static_cast<int>(bounds_.y), 
                     static_cast<int>(bounds_.width), static_cast<int>(bounds_.height));
    
    if (hasImage()) {
        drawImage();
    } else {
        drawPlaceholder();
    }
    
    EndScissorMode();
}

bool Canvas::hasImage() const
{
    return currentTexture_.has_value() && currentTexture_->isValid();
}

void Canvas::setZoom(float zoom)
{
    zoomLevel_ = std::clamp(zoom, MIN_ZOOM, MAX_ZOOM);
}

void Canvas::setPan(Vector2 offset)
{
    panOffset_ = offset;
}

void Canvas::setDrawingTool(DrawingTool tool)
{
    currentTool_ = tool;
    std::cout << "Canvas drawing tool set to: " << static_cast<int>(tool) << std::endl;
}

void Canvas::drawPlaceholder() const
{
    const char* text = "No image loaded. Use 'Load Image' to open a file.";
    int textWidth = MeasureText(text, 20);
    
    float centerX = bounds_.x + bounds_.width / 2 - textWidth / 2;
    float centerY = bounds_.y + bounds_.height / 2 - 10;
    
    DrawText(text, static_cast<int>(centerX), static_cast<int>(centerY), 20, DARKGRAY);
}

void Canvas::resetViewTransform()
{
    zoomLevel_ = 1.0f;
    panOffset_ = {0, 0};
}

Rectangle Canvas::calculateImageDestRect() const
{
    if (!hasImage()) return Rectangle{0, 0, 0, 0};
    
    const float imageWidth = (*currentTexture_)->width * zoomLevel_;
    const float imageHeight = (*currentTexture_)->height * zoomLevel_;
    
    const float imageX = bounds_.x + (bounds_.width - imageWidth) / 2 + panOffset_.x;
    const float imageY = bounds_.y + (bounds_.height - imageHeight) / 2 + panOffset_.y;
    
    return Rectangle{imageX, imageY, imageWidth, imageHeight};
}

Vector2 Canvas::getImageCenter() const
{
    return Vector2{bounds_.x + bounds_.width / 2, bounds_.y + bounds_.height / 2};
}

bool Canvas::hasDrawingLayer() const
{
    return drawingLayer_ && drawingLayer_->isValid();
}

Image Canvas::copyDrawingLayer() const
{
    if (!hasDrawingLayer()) {
        // Return an empty image if no drawing layer exists
        return GenImageColor(1, 1, BLANK);
    }
    
    // Get the texture from the drawing layer and convert to image
    return LoadImageFromTexture((**drawingLayer_).texture);
}

bool Canvas::restoreDrawingLayer(const Image& image)
{
    if (!hasImage()) {
        std::cerr << "Canvas: Cannot restore drawing layer without a base image" << std::endl;
        return false;
    }
    
    try {
        // If drawing layer doesn't exist, initialize it
        if (!hasDrawingLayer()) {
            initializeDrawingLayer();
        }
        
        if (!hasDrawingLayer()) {
            std::cerr << "Canvas: Failed to initialize drawing layer for restoration" << std::endl;
            return false;
        }
        
        // Clear the current drawing layer
        drawingLayer_->clear(BLANK);
        
        // Create a temporary texture from the image and draw it to the drawing layer
        Texture2D tempTexture = LoadTextureFromImage(image);
        
        drawingLayer_->beginDrawing();
        // The drawing layer uses Y-down coordinates, so we don't need to flip here
        // The flip happens during rendering to screen
        DrawTexture(tempTexture, 0, 0, WHITE);
        drawingLayer_->endDrawing();
        
        UnloadTexture(tempTexture);
        
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Canvas: Exception during drawing layer restoration: " << e.what() << std::endl;
        return false;
    }
}

void Canvas::clearDrawingLayer()
{
    if (!hasDrawingLayer()) {
        std::cout << "Canvas: No drawing layer to clear" << std::endl;
        return;
    }
    
    // Clear the drawing layer to transparent
    drawingLayer_->clear(BLANK);
    std::cout << "Canvas: Drawing layer cleared" << std::endl;
}

} // namespace EpiGimp