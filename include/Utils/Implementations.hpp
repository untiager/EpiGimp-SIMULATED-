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

namespace EpiGimp {

// Simple file manager implementation
class SimpleFileManager : public IFileManager {
private:
    std::string lastOpenPath_;
    std::string lastSavePath_;
    
public:
    std::optional<std::string> showOpenDialog(const std::string& filter = "") override {
        // Simplified implementation - in a real app, use native file dialogs
        // For now, check some common locations
        std::vector<std::string> commonPaths = {
            "/home/untiager/delivery/tek3/my_GIMP/png/test.png",
            "test.png",
            "../png/test.png",
            "../../png/test.png"
        };
        
        for (const auto& path : commonPaths) {
            if (fileExists(path)) {
                lastOpenPath_ = path;
                return path;
            }
        }
        
        return std::nullopt;
    }
    
    std::optional<std::string> showSaveDialog(const std::string& filter = "", 
                                            const std::string& defaultName = "") override {
        const std::string savePath = "/home/untiager/delivery/tek3/my_GIMP/png/output.png";
        createDirectories(std::filesystem::path(savePath).parent_path().string());
        lastSavePath_ = savePath;
        return savePath;
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