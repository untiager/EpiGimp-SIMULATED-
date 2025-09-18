//Implementation of the main paint interface

#include "../../include/Class/PaintInterfaceClass.hpp"
#include "../../include/Class/ErrorClass.hpp"
#include <iostream>
#include <filesystem>

// Static member definitions
int PaintInterfaceClass::screenWidth = 1920;
int PaintInterfaceClass::screenHeight = 1080;
bool PaintInterfaceClass::initialized = false;
std::string PaintInterfaceClass::currentImagePath = "";

void PaintInterfaceClass::initialize(int width, int height) {
    if (initialized) return;
    
    screenWidth = width;
    screenHeight = height;
    
    // Initialize window
    InitWindow(screenWidth, screenHeight, "EpiGimp - Paint Interface");
    SetTargetFPS(60);
    
    // Initialize toolbar
    ToolbarClass::initialize(screenWidth, onLoadButtonClick, onSaveButtonClick);
    
    // Initialize canvas (below toolbar)
    int toolbarHeight = ToolbarClass::getToolbarHeight();
    int canvasX = 0;
    int canvasY = toolbarHeight;
    int canvasWidth = screenWidth;
    int canvasHeight = screenHeight - toolbarHeight;
    
    CanvasClass::initialize(canvasX, canvasY, canvasWidth, canvasHeight);
    
    initialized = true;
    std::cout << "Paint interface initialized successfully" << std::endl;
}

void PaintInterfaceClass::run() {
    if (!initialized) return;
    
    while (!WindowShouldClose()) {
        update();
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw();
        EndDrawing();
    }
    
    cleanup();
}

void PaintInterfaceClass::update() {
    if (!initialized) return;
    
    ToolbarClass::update();
    CanvasClass::update();
    handleKeyboardShortcuts();
}

void PaintInterfaceClass::draw() {
    if (!initialized) return;
    
    CanvasClass::draw();
    ToolbarClass::draw();
    
    // Draw status bar at bottom
    int statusY = screenHeight - 25;
    DrawRectangle(0, statusY, screenWidth, 25, LIGHTGRAY);
    DrawLine(0, statusY, screenWidth, statusY, GRAY);
    
    std::string statusText = "Ready";
    if (!currentImagePath.empty()) {
        statusText = "Current image: " + currentImagePath;
    }
    
    DrawText(statusText.c_str(), 10, statusY + 5, 14, BLACK);
}

void PaintInterfaceClass::cleanup() {
    CanvasClass::cleanup();
    CloseWindow();
}

void PaintInterfaceClass::onLoadButtonClick() {
    std::cout << "Load button clicked!" << std::endl;
    
    std::string filePath = showOpenFileDialog();
    if (!filePath.empty()) {
        CanvasClass::loadImage(filePath);
        currentImagePath = filePath;
    }
}

void PaintInterfaceClass::onSaveButtonClick() {
    std::cout << "Save button clicked!" << std::endl;
    
    if (!CanvasClass::hasLoadedImage()) {
        ErrorClass::displayError("No image to save");
        return;
    }
    
    std::string filePath = showSaveFileDialog();
    if (!filePath.empty()) {
        CanvasClass::saveImage(filePath);
    }
}

void PaintInterfaceClass::handleKeyboardShortcuts() {
    // Ctrl+O for Open
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_O)) {
        onLoadButtonClick();
    }
    
    // Ctrl+S for Save
    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
        onSaveButtonClick();
    }
    
    // Escape to close
    if (IsKeyPressed(KEY_ESCAPE)) {
        // Could show a confirmation dialog here
    }
}

std::string PaintInterfaceClass::showOpenFileDialog() {
    // Simplified file selection - in a real implementation you'd use a proper file dialog
    // For now, we'll provide some common image paths or ask user to specify
    
    std::string testImagePath = "../png/test.png";
    if (std::filesystem::exists(testImagePath)) {
        return testImagePath;
    }
    
    // Try some other common locations
    std::vector<std::string> commonPaths = {
        "test.png",
        "image.png",
        "../test.png",
        "../../test.png"
    };
    
    for (const auto& path : commonPaths) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    
    // If no common image found, return empty (in real app, show file browser)
    ErrorClass::displayError("No test image found. Please place an image at ../png/test.png");
    return "";
}

std::string PaintInterfaceClass::showSaveFileDialog() {
    // Simplified save path - in a real implementation you'd use a proper file dialog
    std::string savePath = "/home/untiager/delivery/tek3/my_GIMP/png/output.png";
    
    // Ensure the directory exists
    std::filesystem::create_directories(std::filesystem::path(savePath).parent_path());
    
    return savePath;
}