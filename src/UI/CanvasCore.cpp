//Canvas core functionality
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

Canvas::Canvas(Rectangle bounds, EventDispatcher* dispatcher, HistoryManager* historyManager)
    : bounds_(bounds), zoomLevel_(1.0f), panOffset_{0, 0}, eventDispatcher_(dispatcher),
      historyManager_(historyManager), currentTool_(DrawingTool::None), isDrawing_(false), 
      lastMousePos_{0, 0}, drawingColor_(BLACK), // Initialize with black color
      backgroundVisible_(true), selectedLayerIndex_(-1) // No layer selected initially
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

Image Canvas::copyDrawingImage() const
{
    if (!hasDrawingTexture()) {
        // Return an empty image if no drawing texture exists
        return GenImageColor(1, 1, BLANK);
    }
    
    // Get the texture from the selected drawing layer and convert to image
    const DrawingLayer& layer = drawingLayers_[selectedLayerIndex_];
    Image image = LoadImageFromTexture((**layer.texture).texture);
    
    // RenderTexture images need to be flipped vertically due to coordinate system differences
    ImageFlipVertical(&image);
    
    return image;
}

void Canvas::resetToBackground()
{
    drawingLayers_.clear();
    selectedLayerIndex_ = -1;
    backgroundVisible_ = true;
}

// Multi-layer management methods
void Canvas::setSelectedLayerIndex(int index) {
    if (index >= -1 && index < static_cast<int>(drawingLayers_.size())) {
        selectedLayerIndex_ = index;
        std::cout << "Selected layer index: " << index << std::endl;
    }
}

const DrawingLayer* Canvas::getLayer(int index) const {
    if (index >= 0 && index < static_cast<int>(drawingLayers_.size())) {
        return &drawingLayers_[index];
    }
    return nullptr;
}

DrawingLayer* Canvas::getLayer(int index) {
    if (index >= 0 && index < static_cast<int>(drawingLayers_.size())) {
        return &drawingLayers_[index];
    }
    return nullptr;
}

int Canvas::addNewDrawingLayer(const std::string& name) {
    if (!hasImage()) {
        std::cout << "Cannot add drawing layer: no background image loaded" << std::endl;
        return -1;
    }
    
    std::string layerName = name.empty() ? generateUniqueLayerName() : name;
    drawingLayers_.emplace_back(layerName);
    int newIndex = static_cast<int>(drawingLayers_.size()) - 1;
    
    // Initialize the texture for the new layer
    DrawingLayer& layer = drawingLayers_[newIndex];
    const int width = (*currentTexture_)->width;
    const int height = (*currentTexture_)->height;
    layer.texture = RenderTextureResource(width, height);
    layer.texture->clear(Color{0, 0, 0, 0}); // Clear with transparent
    
    // Select the new layer
    selectedLayerIndex_ = newIndex;
    
    std::cout << "New drawing layer created: " << layerName << " (index " << newIndex << ")" << std::endl;
    return newIndex;
}

void Canvas::deleteLayer(int index) {
    if (index >= 0 && index < static_cast<int>(drawingLayers_.size())) {
        std::string layerName = drawingLayers_[index].name;
        drawingLayers_.erase(drawingLayers_.begin() + index);
        
        // Adjust selected layer index
        if (selectedLayerIndex_ == index) {
            selectedLayerIndex_ = drawingLayers_.empty() ? -1 : std::min(selectedLayerIndex_, static_cast<int>(drawingLayers_.size()) - 1);
        } else if (selectedLayerIndex_ > index) {
            selectedLayerIndex_--;
        }
        
        std::cout << "Deleted layer: " << layerName << std::endl;
    }
}

void Canvas::clearLayer(int index) {
    if (index >= 0 && index < static_cast<int>(drawingLayers_.size())) {
        DrawingLayer& layer = drawingLayers_[index];
        if (layer.texture) {
            layer.texture->clear(Color{0, 0, 0, 0});
            std::cout << "Cleared layer: " << layer.name << std::endl;
        }
    }
}

bool Canvas::isLayerVisible(int index) const {
    const DrawingLayer* layer = getLayer(index);
    return layer ? layer->visible : false;
}

void Canvas::setLayerVisible(int index, bool visible) {
    DrawingLayer* layer = getLayer(index);
    if (layer) {
        layer->visible = visible;
        std::cout << "Layer " << layer->name << " " << (visible ? "shown" : "hidden") << std::endl;
    }
}

const std::string& Canvas::getLayerName(int index) const {
    static const std::string empty = "";
    const DrawingLayer* layer = getLayer(index);
    return layer ? layer->name : empty;
}

// Legacy compatibility methods
bool Canvas::hasDrawingTexture() const {
    return selectedLayerIndex_ >= 0 && selectedLayerIndex_ < static_cast<int>(drawingLayers_.size()) &&
           drawingLayers_[selectedLayerIndex_].texture.has_value();
}

void Canvas::clearDrawingLayer() {
    if (selectedLayerIndex_ >= 0) {
        clearLayer(selectedLayerIndex_);
    }
}

std::string Canvas::generateUniqueLayerName() const {
    // Find the next available layer number
    int nextNumber = 1;
    bool numberExists = true;
    
    while (numberExists) {
        std::string candidateName = "Layer " + std::to_string(nextNumber);
        numberExists = false;
        
        // Check if this name already exists
        for (const auto& layer : drawingLayers_) {
            if (layer.name == candidateName) {
                numberExists = true;
                break;
            }
        }
        
        if (!numberExists) {
            return candidateName;
        }
        nextNumber++;
    }
    
    // Fallback (should never reach here)
    return "Layer " + std::to_string(drawingLayers_.size() + 1);
}

} // namespace EpiGimp