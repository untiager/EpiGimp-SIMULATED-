// Interface declarations for utility classes
#ifndef IMPLEMENTATIONS_HPP
#define IMPLEMENTATIONS_HPP

#include <iostream>
#include <filesystem>
#include <optional>
#include <string>
#include <memory>
#include "raylib.h"
#include "../Core/Interfaces.hpp"
#include "../Core/EventSystem.hpp"

namespace EpiGimp {

class FileBrowser;

class SimpleFileManager : public IFileManager {
private:
    std::unique_ptr<FileBrowser> openBrowser_;
    std::unique_ptr<FileBrowser> saveBrowser_;
    bool showingOpenDialog_;
    bool showingSaveDialog_;
    
public:
    SimpleFileManager();
    ~SimpleFileManager();
    
    std::optional<std::string> showOpenDialog(const std::string& filter = "") override;
    std::optional<std::string> showSaveDialog(const std::string& filter = "", 
                                            const std::string& defaultName = "") override;
    
    std::optional<std::string> updateOpenDialog();
    std::optional<std::string> updateSaveDialog();
    bool isShowingDialog() const;
    
    bool fileExists(const std::string& path) const override;
    bool createDirectories(const std::string& path) const override;
};

class ConsoleErrorHandler : public IErrorHandler {
private:
    EventDispatcher* eventDispatcher_;
    
public:
    explicit ConsoleErrorHandler(EventDispatcher* dispatcher);
    ~ConsoleErrorHandler() = default;
    
    void handleError(const std::string& message) override;
    void handleWarning(const std::string& message) override;
    void handleInfo(const std::string& message) override;
};

class RaylibInputHandler : public IInputHandler {
public:
    RaylibInputHandler() = default;
    ~RaylibInputHandler() = default;
    
    void update() override;
    bool isKeyPressed(int key) const override;
    bool isKeyDown(int key) const override;
    bool isMouseButtonPressed(int button) const override;
    bool isMouseButtonDown(int button) const override;
    Vector2 getMousePosition() const override;
    Vector2 getMouseDelta() const override;
    float getMouseWheelMove() const override;
};

} // namespace EpiGimp

#endif // IMPLEMENTATIONS_HPP