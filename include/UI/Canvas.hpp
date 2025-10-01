//Canvas implementation using RAII and modern C++
#ifndef CANVAS_HPP
#define CANVAS_HPP

#include <optional>
#include <string>
#include <memory>
#include "raylib.h"
#include "../Core/Interfaces.hpp"
#include "../Core/RaylibWrappers.hpp"
#include "../Core/EventSystem.hpp"

namespace EpiGimp {

// Forward declaration
class HistoryManager;

class Canvas : public ICanvas {
private:
    Rectangle bounds_;
    std::optional<TextureResource> currentTexture_;        // Background layer (loaded image)
    std::optional<RenderTextureResource> drawingTexture_;  // Drawing layer (transparent overlay)
    std::string currentImagePath_;
    float zoomLevel_;
    Vector2 panOffset_;
    EventDispatcher* eventDispatcher_;
    HistoryManager* historyManager_;  // For undo/redo functionality
    
    // Simple layer system
    bool backgroundVisible_;
    bool drawingVisible_;
    
    // Drawing state
    DrawingTool currentTool_;
    bool isDrawing_;
    Vector2 lastMousePos_;
    Color drawingColor_; // Current drawing color
    
    static constexpr float MIN_ZOOM = 0.1f;
    static constexpr float MAX_ZOOM = 5.0f;
    static constexpr float ZOOM_STEP = 0.1f;
    static constexpr float PAN_SPEED = 2.0f;

public:
    explicit Canvas(Rectangle bounds, EventDispatcher* dispatcher, HistoryManager* historyManager = nullptr);
    ~Canvas() override = default;

    // Non-copyable but moveable
    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;
    Canvas(Canvas&&) = default;
    Canvas& operator=(Canvas&&) = default;

    // IUIComponent interface
    void update(float deltaTime) override;
    void draw() const override;
    Rectangle getBounds() const override { return bounds_; }

    // ICanvas interface
    void loadImage(const std::string& filePath) override;
    bool saveImage(const std::string& filePath) override;
    bool hasImage() const override;
    void setZoom(float zoom) override;
    float getZoom() const override { return zoomLevel_; }
    void setPan(Vector2 offset) override;
    Vector2 getPan() const override { return panOffset_; }
    void setDrawingTool(DrawingTool tool) override;
    
    // Simple layer management
    bool isBackgroundVisible() const { return backgroundVisible_; }
    bool isDrawingVisible() const { return drawingVisible_; }
    void setBackgroundVisible(bool visible) { backgroundVisible_ = visible; }
    void setDrawingVisible(bool visible) { drawingVisible_ = visible; }
    void clearDrawingLayer();
    void resetToBackground();
    void addNewDrawingLayer(); // Create a new drawing layer (overwrites current)
    void deleteDrawingLayer(); // Remove the drawing layer entirely
    
    // History/Command support
    bool hasDrawingTexture() const;
    Image copyDrawingImage() const;
    void initializeDrawingTexture(); // Initialize drawing texture

private:
    void handleInput();
    void handleZoom();
    void handlePanning();
    void handleDrawing();  // New method for drawing input
    void drawImage() const;
    void drawPlaceholder() const;
    void drawStroke(Vector2 from, Vector2 to);
    void onColorChanged(const ColorChangedEvent& event); // Handle color change events
    Rectangle calculateImageDestRect() const;
    Vector2 getImageCenter() const;
    std::optional<TextureResource> createTextureFromFile(const std::string& filePath);
    void resetViewTransform();
};

} // namespace EpiGimp

#endif // CANVAS_HPP