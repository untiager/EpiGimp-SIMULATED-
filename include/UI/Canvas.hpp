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

class Canvas : public ICanvas {
private:
    Rectangle bounds_;
    std::optional<TextureResource> currentTexture_;
    std::string currentImagePath_;
    float zoomLevel_;
    Vector2 panOffset_;
    EventDispatcher* eventDispatcher_;
    
    static constexpr float MIN_ZOOM = 0.1f;
    static constexpr float MAX_ZOOM = 5.0f;
    static constexpr float ZOOM_STEP = 0.1f;
    static constexpr float PAN_SPEED = 2.0f;

public:
    explicit Canvas(Rectangle bounds, EventDispatcher* dispatcher);
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

private:
    void handleInput();
    void handleZoom();
    void handlePanning();
    void drawImage() const;
    void drawPlaceholder() const;
    Rectangle calculateImageDestRect() const;
    Vector2 getImageCenter() const;
    std::optional<TextureResource> createTextureFromFile(const std::string& filePath);
    void resetViewTransform();
};

} // namespace EpiGimp

#endif // CANVAS_HPP