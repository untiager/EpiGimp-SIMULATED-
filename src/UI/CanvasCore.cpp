//Canvas core functionality
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

Canvas::Canvas(Rectangle bounds, EventDispatcher* dispatcher, HistoryManager* historyManager)
    : bounds_(bounds), zoomLevel_(1.0f), panOffset_{0, 0}, eventDispatcher_(dispatcher),
      historyManager_(historyManager), currentTool_(DrawingTool::None), isDrawing_(false), 
      lastMousePos_{0, 0}, drawingColor_(BLACK), // Initialize with black color
      backgroundVisible_(true), drawingVisible_(true) // Both layers visible by default
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
    
    float imageWidth = 0;
    float imageHeight = 0;
    
    // Get dimensions from current texture
    if (currentTexture_) {
        imageWidth = (*currentTexture_)->width * zoomLevel_;
        imageHeight = (*currentTexture_)->height * zoomLevel_;
    }
    
    const float imageX = bounds_.x + (bounds_.width - imageWidth) / 2 + panOffset_.x;
    const float imageY = bounds_.y + (bounds_.height - imageHeight) / 2 + panOffset_.y;
    
    return Rectangle{imageX, imageY, imageWidth, imageHeight};
}

Vector2 Canvas::getImageCenter() const
{
    return Vector2{bounds_.x + bounds_.width / 2, bounds_.y + bounds_.height / 2};
}

bool Canvas::hasDrawingTexture() const
{
    return drawingTexture_ && drawingTexture_->isValid();
}

Image Canvas::copyDrawingImage() const
{
    if (!hasDrawingTexture()) {
        // Return an empty image if no drawing texture exists
        return GenImageColor(1, 1, BLANK);
    }
    
    // Get the texture from the drawing layer and convert to image
    Image image = LoadImageFromTexture((**drawingTexture_).texture);
    
    // RenderTexture images need to be flipped vertically due to coordinate system differences
    ImageFlipVertical(&image);
    
    return image;
}

void Canvas::clearDrawingLayer()
{
    if (hasDrawingTexture()) {
        drawingTexture_->clear(Color{0, 0, 0, 0}); // Clear with transparent
    }
}

void Canvas::resetToBackground()
{
    clearDrawingLayer();
    backgroundVisible_ = true;
    drawingVisible_ = true;
}

} // namespace EpiGimp