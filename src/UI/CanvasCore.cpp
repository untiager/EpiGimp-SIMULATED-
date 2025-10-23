//Canvas core functionality
#include "../../include/UI/Canvas.hpp"
#include "../../include/Commands/DeleteSelectionCommand.hpp"
#include "../../include/Commands/FlipSelectionCommands.hpp"
#include "../../include/Core/HistoryManager.hpp"
#include "rlgl.h"  // For low-level OpenGL blend functions
#include <iostream>
#include <cmath>

namespace EpiGimp {

Canvas::Canvas(Rectangle bounds, EventDispatcher* dispatcher, HistoryManager* historyManager, bool autoCreateBlankCanvas)
    : bounds_(bounds), zoomLevel_(1.0f), panOffset_{0, 0}, eventDispatcher_(dispatcher),
      historyManager_(historyManager), currentTool_(DrawingTool::None), isDrawing_(false), 
      lastMousePos_{0, 0}, primaryColor_(BLACK), secondaryColor_(WHITE), drawingColor_(BLACK), // Initialize with black primary, white secondary
      mirrorModeEnabled_(false),
      isSelecting_(false), hasSelection_(false), selectionStart_{0, 0}, selectionEnd_{0, 0}, 
      selectionRect_{0, 0, 0, 0}, selectionAnimTime_(0.0f),
      isResizingSelection_(false), resizeHandle_(ResizeHandle::None), resizeStartPos_{0, 0}, resizeStartRect_{0, 0, 0, 0},
      isTransformMode_(false), isTransformingContent_(false), contentOriginalRect_{0, 0, 0, 0}, contentTransformRect_{0, 0, 0, 0},
      backgroundVisible_(true), canvasFlippedVertical_(false), canvasFlippedHorizontal_(false), 
      selectedLayerIndex_(-1) // No layer selected initially
{
    
    if (!dispatcher)
        throw std::invalid_argument("EventDispatcher cannot be null");
    
    // Subscribe to color events
    dispatcher->subscribe<ColorChangedEvent>([this](const ColorChangedEvent& event) {
        onColorChanged(event);
    });
    
    dispatcher->subscribe<PrimaryColorChangedEvent>([this](const PrimaryColorChangedEvent& event) {
        onPrimaryColorChanged(event);
    });
    
    dispatcher->subscribe<SecondaryColorChangedEvent>([this](const SecondaryColorChangedEvent& event) {
        onSecondaryColorChanged(event);
    });
    
    if (autoCreateBlankCanvas)
        createBlankCanvas(800, 600, WHITE);
    
    std::cout << "Canvas initialized with bounds: " 
              << bounds.x << ", " << bounds.y << ", " 
              << bounds.width << ", " << bounds.height << std::endl;
}

void Canvas::update(float deltaTime)
{
    handleInput();
    handleDrawing();
    handleSelection();
    
    // Update selection animation
    selectionAnimTime_ += deltaTime * 2.0f; // Speed up animation
    if (selectionAnimTime_ > 2.0f * PI) {
        selectionAnimTime_ = 0.0f;
    }
}

void Canvas::draw() const
{
    // Use a light gray background to distinguish from white drawing area
    DrawRectangleRec(bounds_, Color{240, 240, 240, 255}); // Light gray background
    DrawRectangleLinesEx(bounds_, 1, DARKGRAY);
    
    BeginScissorMode(static_cast<int>(bounds_.x), static_cast<int>(bounds_.y), 
                     static_cast<int>(bounds_.width), static_cast<int>(bounds_.height));
    
    if (hasImage()) {
        drawImage();
    } else {
        drawPlaceholder();
    }
    
    // Draw selection if active
    if (hasSelection_ || isSelecting_) {
        drawSelection();
    }
    
    // Draw mirror mode indicator line
    if (mirrorModeEnabled_ && hasImage()) {
        const Rectangle imageRect = calculateImageDestRect();
        const float centerX = imageRect.x + imageRect.width / 2.0f;
        DrawLine(static_cast<int>(centerX), static_cast<int>(imageRect.y), 
                 static_cast<int>(centerX), static_cast<int>(imageRect.y + imageRect.height),
                 Color{255, 0, 0, 128}); // Semi-transparent red line
    }
    
    EndScissorMode();
    
    // Draw zoom indicator outside scissor mode
    drawZoomIndicator();
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
    const char* text = "Canvas ready for drawing. Select a tool to start!";
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
    if (!hasDrawingTexture())
        return GenImageColor(1, 1, BLANK);
    
    const DrawingLayer& layer = drawingLayers_[selectedLayerIndex_];
    Image image = LoadImageFromTexture((**layer.texture).texture);
    
    ImageFlipVertical(&image);
    
    return image;
}

void Canvas::resetToBackground()
{
    drawingLayers_.clear();
    selectedLayerIndex_ = -1;
    backgroundVisible_ = true;
}

void Canvas::setSelectedLayerIndex(int index)
{
    if (index >= -1 && index < static_cast<int>(drawingLayers_.size())) {
        selectedLayerIndex_ = index;
        std::cout << "Selected layer index: " << index << std::endl;
    }
}

const DrawingLayer* Canvas::getLayer(int index) const
{
    if (index >= 0 && index < static_cast<int>(drawingLayers_.size()))
        return &drawingLayers_[index];
    return nullptr;
}

DrawingLayer* Canvas::getLayer(int index)
{
    if (index >= 0 && index < static_cast<int>(drawingLayers_.size()))
        return &drawingLayers_[index];
    return nullptr;
}

int Canvas::addNewDrawingLayer(const std::string& name)
{
    if (!hasImage()) {
        std::cout << "Cannot add drawing layer: no background image loaded" << std::endl;
        return -1;
    }
    
    std::string layerName = name.empty() ? generateUniqueLayerName() : name;
    drawingLayers_.emplace_back(layerName);
    int newIndex = static_cast<int>(drawingLayers_.size()) - 1;
    
    DrawingLayer& layer = drawingLayers_[newIndex];
    const int width = (*currentTexture_)->width;
    const int height = (*currentTexture_)->height;
    layer.texture = RenderTextureResource(width, height);
    layer.texture->clear(Color{0, 0, 0, 0}); // Clear with transparent
    
    selectedLayerIndex_ = newIndex;
    
    std::cout << "New drawing layer created: " << layerName << " (index " << newIndex << ")" << std::endl;
    return newIndex;
}

void Canvas::deleteLayer(int index)
{
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

void Canvas::clearLayer(int index)
{
    if (index >= 0 && index < static_cast<int>(drawingLayers_.size())) {
        DrawingLayer& layer = drawingLayers_[index];
        if (layer.texture) {
            layer.texture->clear(Color{0, 0, 0, 0});
            std::cout << "Cleared layer: " << layer.name << std::endl;
        }
    }
}

void Canvas::moveLayer(int fromIndex, int toIndex)
{
    if (fromIndex >= 0 && fromIndex < static_cast<int>(drawingLayers_.size()) &&
        toIndex >= 0 && toIndex < static_cast<int>(drawingLayers_.size()) &&
        fromIndex != toIndex) {
        
        DrawingLayer layer = std::move(drawingLayers_[fromIndex]);
        drawingLayers_.erase(drawingLayers_.begin() + fromIndex);
        drawingLayers_.insert(drawingLayers_.begin() + toIndex, std::move(layer));
        
        // Adjust selected layer index
        if (selectedLayerIndex_ == fromIndex) {
            selectedLayerIndex_ = toIndex;
        } else if (fromIndex < toIndex && selectedLayerIndex_ > fromIndex && selectedLayerIndex_ <= toIndex) {
            selectedLayerIndex_--;
        } else if (fromIndex > toIndex && selectedLayerIndex_ >= toIndex && selectedLayerIndex_ < fromIndex) {
            selectedLayerIndex_++;
        }
        
        std::cout << "Moved layer from index " << fromIndex << " to " << toIndex << std::endl;
    }
}

bool Canvas::isLayerVisible(int index) const
{
    const DrawingLayer* layer = getLayer(index);
    return layer ? layer->visible : false;
}

void Canvas::setLayerVisible(int index, bool visible)
{
    DrawingLayer* layer = getLayer(index);
    if (layer) {
        layer->visible = visible;
        std::cout << "Layer " << layer->name << " " << (visible ? "shown" : "hidden") << std::endl;
    }
}

const std::string& Canvas::getLayerName(int index) const
{
    static const std::string empty = "";
    const DrawingLayer* layer = getLayer(index);
    return layer ? layer->name : empty;
}

bool Canvas::hasDrawingTexture() const
{
    return selectedLayerIndex_ >= 0 && selectedLayerIndex_ < static_cast<int>(drawingLayers_.size()) &&
           drawingLayers_[selectedLayerIndex_].texture.has_value();
}

void Canvas::clearDrawingLayer()
{
    if (selectedLayerIndex_ >= 0)
        clearLayer(selectedLayerIndex_);
}

std::string Canvas::generateUniqueLayerName() const
{
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
        
        if (!numberExists)
            return candidateName;
        nextNumber++;
    }
    
    // Fallback
    return "Layer " + std::to_string(drawingLayers_.size() + 1);
}

void Canvas::clearSelection()
{
    hasSelection_ = false;
    isSelecting_ = false;
    isResizingSelection_ = false;
    resizeHandle_ = ResizeHandle::None;
    selectionRect_ = Rectangle{0, 0, 0, 0};
    std::cout << "Selection cleared" << std::endl;
}

void Canvas::selectAll()
{
    if (!hasImage()) return;
    
    hasSelection_ = true;
    isSelecting_ = false;
    selectionRect_ = Rectangle{0, 0, static_cast<float>((*currentTexture_)->width), static_cast<float>((*currentTexture_)->height)};
    std::cout << "Selected all (" << selectionRect_.width << "x" << selectionRect_.height << ")" << std::endl;
}

Vector2 Canvas::screenToImageCoords(Vector2 screenPos) const
{
    if (!hasImage()) return Vector2{0, 0};
    
    const Rectangle imageRect = calculateImageDestRect();
    
    return Vector2{
        (screenPos.x - imageRect.x) / imageRect.width * (*currentTexture_)->width,
        (screenPos.y - imageRect.y) / imageRect.height * (*currentTexture_)->height
    };
}

Vector2 Canvas::imageToScreenCoords(Vector2 imagePos) const
{
    if (!hasImage()) return Vector2{0, 0};
    
    const Rectangle imageRect = calculateImageDestRect();
    
    return Vector2{
        imageRect.x + (imagePos.x / (*currentTexture_)->width) * imageRect.width,
        imageRect.y + (imagePos.y / (*currentTexture_)->height) * imageRect.height
    };
}

Rectangle Canvas::normalizeRect(Vector2 start, Vector2 end) const
{
    const float x = std::min(start.x, end.x);
    const float y = std::min(start.y, end.y);
    const float width = std::abs(end.x - start.x);
    const float height = std::abs(end.y - start.y);
    
    return Rectangle{x, y, width, height};
}

void Canvas::deleteSelection()
{
    if (!hasSelection_ || !hasDrawingTexture()) {
        std::cout << "Cannot delete: no selection or no drawing texture" << std::endl;
        return;
    }
    
    DrawingLayer& layer = drawingLayers_[selectedLayerIndex_];
    if (!layer.visible) {
        std::cout << "Cannot delete: layer is not visible" << std::endl;
        return;
    }
    
    // Use command pattern for undo/redo support if history manager is available
    if (historyManager_) {
        // Create and execute the delete selection command
        auto command = createDeleteSelectionCommand(this, "Delete Selection");
        if (command && historyManager_->executeCommand(std::move(command))) {
            std::cout << "Delete selection completed and added to history" << std::endl;
        } else {
            std::cout << "Failed to execute delete selection command" << std::endl;
        }
    } else {
        // Fallback: direct deletion without history
        deleteSelectionInternal();
    }
}

void Canvas::deleteSelectionInternal()
{
    if (!hasSelection_ || !hasDrawingTexture()) {
        return;
    }
    
    DrawingLayer& layer = drawingLayers_[selectedLayerIndex_];
    if (!layer.visible) {
        return;
    }
    
    // Clear the selection area by drawing a transparent rectangle with proper clearing
    layer.texture->beginDrawing();
    
    // Use scissor test to limit clearing to the selection area
    BeginScissorMode((int)selectionRect_.x, (int)selectionRect_.y, 
                     (int)selectionRect_.width, (int)selectionRect_.height);
    
    // Clear the background to transparent in the scissored area
    ClearBackground(Color{0, 0, 0, 0});
    
    EndScissorMode();
    
    layer.texture->endDrawing();
    
    std::cout << "Deleted selection area: (" << selectionRect_.x << "," << selectionRect_.y 
              << ") " << selectionRect_.width << "x" << selectionRect_.height 
              << " on layer: " << layer.name << std::endl;
    
    // Clear the selection after deletion
    clearSelection();
}

void Canvas::deleteSelectionWithCommand()
{
    deleteSelection(); // Use the main deleteSelection method which handles commands properly
}

void Canvas::flipSelectionVertical()
{
    if (!hasSelection_ || !hasDrawingTexture() || !historyManager_) {
        return;
    }
    
    auto command = createFlipSelectionVerticalCommand(this);
    historyManager_->executeCommand(std::move(command));
    
    std::cout << "Executed vertical flip command with history support" << std::endl;
}

void Canvas::flipSelectionHorizontal()
{
    if (!hasSelection_ || !hasDrawingTexture() || !historyManager_) {
        return;
    }
    
    auto command = createFlipSelectionHorizontalCommand(this);
    historyManager_->executeCommand(std::move(command));
    
    std::cout << "Executed horizontal flip command with history support" << std::endl;
}

void Canvas::drawZoomIndicator() const
{
    // Draw zoom indicator in the bottom-right corner of the canvas
    const float indicatorWidth = 80.0f;
    const float indicatorHeight = 25.0f;
    const float margin = 10.0f;
    
    Rectangle indicatorRect = {
        bounds_.x + bounds_.width - indicatorWidth - margin,
        bounds_.y + bounds_.height - indicatorHeight - margin,
        indicatorWidth,
        indicatorHeight
    };
    
    // Background with semi-transparent black
    DrawRectangleRec(indicatorRect, Color{0, 0, 0, 150});
    DrawRectangleLinesEx(indicatorRect, 1.0f, LIGHTGRAY);
    
    // Format zoom percentage
    char zoomText[16];
    int zoomPercent = static_cast<int>(zoomLevel_ * 100);
    snprintf(zoomText, sizeof(zoomText), "%d%%", zoomPercent);
    
    // Calculate text position (centered)
    Vector2 textSize = MeasureTextEx(GetFontDefault(), zoomText, 12, 1);
    Vector2 textPos = {
        indicatorRect.x + (indicatorRect.width - textSize.x) / 2,
        indicatorRect.y + (indicatorRect.height - textSize.y) / 2
    };
    
    // Draw text
    DrawTextEx(GetFontDefault(), zoomText, textPos, 12, 1, WHITE);
}

} // namespace EpiGimp