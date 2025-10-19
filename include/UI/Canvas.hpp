//Canvas implementation using RAII and modern C++
#ifndef CANVAS_HPP
#define CANVAS_HPP

#include <optional>
#include <string>
#include <memory>
#include <vector>
#include "raylib.h"
#include "../Core/Interfaces.hpp"
#include "../Core/RaylibWrappers.hpp"
#include "../Core/EventSystem.hpp"

namespace EpiGimp {

struct DrawingLayer {
    std::optional<RenderTextureResource> texture;
    bool visible;
    std::string name;
    
    DrawingLayer(const std::string& layerName) : visible(true), name(layerName) {}
};

class HistoryManager;

class Canvas : public ICanvas {
private:
    Rectangle bounds_;
    std::optional<TextureResource> currentTexture_;        // Background layer (loaded image)
    std::vector<DrawingLayer> drawingLayers_;              // Multiple drawing layers
    std::string currentImagePath_;
    float zoomLevel_;
    Vector2 panOffset_;
    EventDispatcher* eventDispatcher_;
    HistoryManager* historyManager_;                       // For undo/redo functionality
    
    DrawingTool currentTool_;
    bool isDrawing_;
    Vector2 lastMousePos_;
    Color primaryColor_;                                   // Primary drawing color (left-click)
    Color secondaryColor_;                                 // Secondary drawing color (right-click)
    Color drawingColor_;                                   // Current drawing color (deprecated, for compatibility)
    
    // Selection state
    bool isSelecting_;                                     // True when actively making a selection
    bool hasSelection_;                                    // True when a selection exists
    Vector2 selectionStart_;                               // Selection start point (screen coordinates)
    Vector2 selectionEnd_;                                 // Selection end point (screen coordinates)
    Rectangle selectionRect_;                              // Current selection rectangle (image coordinates)
    float selectionAnimTime_;                              // For marching ants animation
    
    bool backgroundVisible_;
    int selectedLayerIndex_;                               // Currently selected layer for drawing/editing
    
    static constexpr float MIN_ZOOM = 0.1f;
    static constexpr float MAX_ZOOM = 5.0f;
    static constexpr float ZOOM_STEP = 0.1f;
    static constexpr float PAN_SPEED = 2.0f;

public:
    explicit Canvas(Rectangle bounds, EventDispatcher* dispatcher, HistoryManager* historyManager = nullptr, bool autoCreateBlankCanvas = true);
    ~Canvas() override = default;

    Canvas(const Canvas&) = delete;
    Canvas& operator=(const Canvas&) = delete;
    Canvas(Canvas&&) = default;
    Canvas& operator=(Canvas&&) = default;

    void update(float deltaTime) override;
    void draw() const override;
    Rectangle getBounds() const override { return bounds_; }

    void loadImage(const std::string& filePath) override;
    void createBlankCanvas(int width = 800, int height = 600, Color backgroundColor = WHITE);
    bool saveImage(const std::string& filePath) override;
    bool hasImage() const override;
    void setZoom(float zoom) override;
    float getZoom() const override { return zoomLevel_; }
    void setPan(Vector2 offset) override;
    Vector2 getPan() const override { return panOffset_; }
    void setDrawingTool(DrawingTool tool) override;
    
    bool isBackgroundVisible() const { return backgroundVisible_; }
    void setBackgroundVisible(bool visible) { backgroundVisible_ = visible; }
    
    int getLayerCount() const { return static_cast<int>(drawingLayers_.size()); }
    int getSelectedLayerIndex() const { return selectedLayerIndex_; }
    void setSelectedLayerIndex(int index);
    const DrawingLayer* getLayer(int index) const;
    DrawingLayer* getLayer(int index);
    
    int addNewDrawingLayer(const std::string& name = "");
    void deleteLayer(int index);
    void clearLayer(int index);
    void moveLayer(int fromIndex, int toIndex);  // Move layer to new position
    bool isLayerVisible(int index) const;
    void setLayerVisible(int index, bool visible);
    const std::string& getLayerName(int index) const;
    
    bool hasDrawingTexture() const;
    void clearDrawingLayer();  // Clear selected layer
    void resetToBackground();
    
    Image copyDrawingImage() const;  // Copy selected layer
    void initializeDrawingTexture(); // Initialize selected layer texture
    
    // Selection methods
    bool hasSelection() const { return hasSelection_; }
    Rectangle getSelectionRect() const { return selectionRect_; }
    void clearSelection();
    void selectAll();

private:
    void handleInput();
    void handleZoom();
    void handlePanning();
    void handleDrawing();  // New method for drawing input
    void handleSelection(); // New method for selection input
    void drawImage() const;
    void drawPlaceholder() const;
    void drawSelection() const; // Draw selection rectangle with marching ants
    void drawStroke(Vector2 from, Vector2 to);
    void onColorChanged(const ColorChangedEvent& event); // Handle color change events (deprecated)
    void onPrimaryColorChanged(const PrimaryColorChangedEvent& event); // Handle primary color change
    void onSecondaryColorChanged(const SecondaryColorChangedEvent& event); // Handle secondary color change
    Rectangle calculateImageDestRect() const;
    Vector2 getImageCenter() const;
    std::optional<TextureResource> createTextureFromFile(const std::string& filePath);
    void resetViewTransform();
    std::string generateUniqueLayerName() const; // Generate unique layer name
    Vector2 screenToImageCoords(Vector2 screenPos) const; // Convert screen coords to image coords
    Vector2 imageToScreenCoords(Vector2 imagePos) const;  // Convert image coords to screen coords
    Rectangle normalizeRect(Vector2 start, Vector2 end) const; // Create normalized rectangle from two points
    void drawMarchingAnts(Rectangle rect, Color color, float dashLength, float offset) const; // Draw marching ants effect
    void drawDashedLine(Vector2 start, Vector2 end, Color color, float dashLength, float offset) const; // Draw dashed line
};

} // namespace EpiGimp

#endif // CANVAS_HPP