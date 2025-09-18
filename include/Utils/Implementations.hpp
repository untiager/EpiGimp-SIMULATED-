//Concrete implementations of utility interfaces
#ifndef CONCRETE_IMPLEMENTATIONS_HPP
#define CONCRETE_IMPLEMENTATIONS_HPP

#include <iostream>
#include <filesystem>
#include <optional>
#include <string>
#include "raylib.h"
#include "../Core/Interfaces.hpp"
#include "../Core/EventSystem.hpp"
#include "FileBrowser.hpp"

namespace EpiGimp {

// Simple file manager implementation
class SimpleFileManager : public IFileManager {
private:
    std::unique_ptr<FileBrowser> openBrowser_;
    std::unique_ptr<FileBrowser> saveBrowser_;
    bool showingOpenDialog_;
    bool showingSaveDialog_;
    
public:
    SimpleFileManager() 
        : showingOpenDialog_(false)
        , showingSaveDialog_(false) 
    {
        openBrowser_ = std::make_unique<FileBrowser>();
        saveBrowser_ = std::make_unique<FileBrowser>();
    }
    
    std::optional<std::string> showOpenDialog(const std::string& filter = "") override {
        showingOpenDialog_ = true;
        openBrowser_->reset();
        return std::nullopt; // Will be handled in update loop
    }
    
    std::optional<std::string> showSaveDialog(const std::string& filter = "", 
                                            const std::string& defaultName = "") override {
        showingSaveDialog_ = true;
        saveBrowser_->reset();
        return std::nullopt; // Will be handled in update loop
    }
    
    // New methods for rendering the dialogs
    std::optional<std::string> updateOpenDialog() {
        if (!showingOpenDialog_) return std::nullopt;
        
        float screenWidth = (float)GetScreenWidth();
        float screenHeight = (float)GetScreenHeight();
        float dialogWidth = screenWidth * 0.8f;
        float dialogHeight = screenHeight * 0.8f;
        float x = (screenWidth - dialogWidth) / 2;
        float y = (screenHeight - dialogHeight) / 2;
        
        // Semi-transparent background
        DrawRectangle(0, 0, (int)screenWidth, (int)screenHeight, ColorAlpha(BLACK, 0.7f));
        
        bool result = openBrowser_->renderOpenDialog(x, y, dialogWidth, dialogHeight);
        
        if (result) {
            showingOpenDialog_ = false;
            return openBrowser_->getSelectedFile();
        }
        
        // Check for ESC key to cancel
        if (IsKeyPressed(KEY_ESCAPE)) {
            showingOpenDialog_ = false;
            return std::nullopt;
        }
        
        return std::nullopt;
    }
    
    std::optional<std::string> updateSaveDialog() {
        if (!showingSaveDialog_) return std::nullopt;
        
        float screenWidth = (float)GetScreenWidth();
        float screenHeight = (float)GetScreenHeight();
        float dialogWidth = screenWidth * 0.8f;
        float dialogHeight = screenHeight * 0.8f;
        float x = (screenWidth - dialogWidth) / 2;
        float y = (screenHeight - dialogHeight) / 2;
        
        // Semi-transparent background
        DrawRectangle(0, 0, (int)screenWidth, (int)screenHeight, ColorAlpha(BLACK, 0.7f));
        
        bool result = saveBrowser_->renderSaveDialog(x, y, dialogWidth, dialogHeight);
        
        if (result) {
            showingSaveDialog_ = false;
            std::string filename = saveBrowser_->getSaveFileName();
            return filename.empty() ? std::nullopt : std::make_optional(filename);
        }
        
        // Check for ESC key to cancel
        if (IsKeyPressed(KEY_ESCAPE)) {
            showingSaveDialog_ = false;
            return std::nullopt;
        }
        
        return std::nullopt;
    }
    
    bool isShowingDialog() const {
        return showingOpenDialog_ || showingSaveDialog_;
    }
    
    bool fileExists(const std::string& path) const override {
        return std::filesystem::exists(path);
    }
    
    bool createDirectories(const std::string& path) const override {
        std::error_code ec;
        return std::filesystem::create_directories(path, ec);
    }
};

// Console-based error handler
class ConsoleErrorHandler : public IErrorHandler {
private:
    EventDispatcher* eventDispatcher_;
    
public:
    explicit ConsoleErrorHandler(EventDispatcher* dispatcher) 
        : eventDispatcher_(dispatcher) {}
    
    void handleError(const std::string& message) override {
        std::cerr << "[ERROR] " << message << std::endl;
        if (eventDispatcher_) {
            eventDispatcher_->emit<ErrorEvent>(message);
        }
    }
    
    void handleWarning(const std::string& message) override {
        std::cout << "[WARNING] " << message << std::endl;
    }
    
    void handleInfo(const std::string& message) override {
        std::cout << "[INFO] " << message << std::endl;
    }
};

// Raylib-based input handler
class RaylibInputHandler : public IInputHandler {
public:
    void update() override {
        // Raylib handles input automatically, so nothing to do here
    }
    
    bool isKeyPressed(int key) const override {
        return IsKeyPressed(key);
    }
    
    bool isKeyDown(int key) const override {
        return IsKeyDown(key);
    }
    
    bool isMouseButtonPressed(int button) const override {
        return IsMouseButtonPressed(button);
    }
    
    bool isMouseButtonDown(int button) const override {
        return IsMouseButtonDown(button);
    }
    
    Vector2 getMousePosition() const override {
        return GetMousePosition();
    }
    
    Vector2 getMouseDelta() const override {
        return GetMouseDelta();
    }
    
    float getMouseWheelMove() const override {
        return GetMouseWheelMove();
    }
};

} // namespace EpiGimp

#endif // CONCRETE_IMPLEMENTATIONS_HPP