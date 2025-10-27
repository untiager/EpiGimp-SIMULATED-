//Interfaces for better abstraction and testability
#ifndef INTERFACES_HPP
#define INTERFACES_HPP

#include <string>
#include <optional>
#include "raylib.h"
#include "EventSystem.hpp"

namespace EpiGimp {

class TextureResource;
class ImageResource;

enum class DrawingTool {
    None,
    Crayon,
    Brush,
    Airbrush,
    Ink,
    Blur,
    Burn,
    Select,
    Mirror,
    Eyedropper
};

class IUIComponent {
public:
    virtual ~IUIComponent() = default;
    virtual void update(float deltaTime) = 0;
    virtual void draw() const = 0;
    virtual Rectangle getBounds() const = 0;
};

class IToolbar : public IUIComponent {
public:
    virtual ~IToolbar() = default;
    virtual void addButton(const std::string& text, std::function<void()> onClick) = 0;
    virtual int getHeight() const = 0;
};

class ICanvas : public IUIComponent {
public:
    virtual ~ICanvas() = default;
    virtual void loadImage(const std::string& filePath) = 0;
    virtual bool saveImage(const std::string& filePath) = 0;
    virtual bool hasImage() const = 0;
    virtual void setZoom(float zoom) = 0;
    virtual float getZoom() const = 0;
    virtual void setPan(Vector2 offset) = 0;
    virtual Vector2 getPan() const = 0;
    virtual void setDrawingTool(DrawingTool tool) = 0;
    virtual void flipLayerVertical(int index = -1) = 0;
};

class IFileManager {
public:
    virtual ~IFileManager() = default;
    virtual std::optional<std::string> showOpenDialog(const std::string& filter = "") = 0;
    virtual std::optional<std::string> showSaveDialog(const std::string& filter = "", 
                                                      const std::string& defaultName = "") = 0;
    virtual bool fileExists(const std::string& path) const = 0;
    virtual bool createDirectories(const std::string& path) const = 0;
};

class IErrorHandler {
public:
    virtual ~IErrorHandler() = default;
    virtual void handleError(const std::string& message) = 0;
    virtual void handleWarning(const std::string& message) = 0;
    virtual void handleInfo(const std::string& message) = 0;
};

class IInputHandler {
public:
    virtual ~IInputHandler() = default;
    virtual void update() = 0;
    virtual bool isKeyPressed(int key) const = 0;
    virtual bool isKeyDown(int key) const = 0;
    virtual bool isMouseButtonPressed(int button) const = 0;
    virtual bool isMouseButtonDown(int button) const = 0;
    virtual Vector2 getMousePosition() const = 0;
    virtual Vector2 getMouseDelta() const = 0;
    virtual float getMouseWheelMove() const = 0;
};

} // namespace EpiGimp

#endif // INTERFACES_HPP