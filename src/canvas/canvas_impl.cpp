//Implementation of the canvas for image editing

#include "../../include/Class/CanvasClass.hpp"
#include "../../include/Class/ErrorClass.hpp"
#include <iostream>

// Static member definitions
Texture2D CanvasClass::currentTexture = {0};
bool CanvasClass::hasImage = false;
Rectangle CanvasClass::canvasArea = {0, 0, 0, 0};
float CanvasClass::zoomLevel = 1.0f;
Vector2 CanvasClass::panOffset = {0, 0};

void CanvasClass::initialize(int x, int y, int width, int height) {
    canvasArea = Rectangle{(float)x, (float)y, (float)width, (float)height};
    zoomLevel = 1.0f;
    panOffset = {0, 0};
    hasImage = false;
    std::cout << "Canvas initialized at (" << x << ", " << y << ") with size " << width << "x" << height << std::endl;
}

void CanvasClass::loadImage(const std::string& imagePath) {
    // Clean up previous texture if exists
    if (hasImage && currentTexture.id != 0) {
        UnloadTexture(currentTexture);
    }
    
    Image image = LoadImage(imagePath.c_str());
    if (image.data == nullptr) {
        ErrorClass::displayError("Failed to load image: " + imagePath);
        hasImage = false;
        return;
    }
    
    // Resize image if too large for canvas
    const int maxWidth = (int)canvasArea.width;
    const int maxHeight = (int)canvasArea.height;
    
    if (image.width > maxWidth || image.height > maxHeight) {
        float widthRatio = (float)maxWidth / image.width;
        float heightRatio = (float)maxHeight / image.height;
        float scale = (widthRatio < heightRatio) ? widthRatio : heightRatio;
        int newWidth = (int)(image.width * scale);
        int newHeight = (int)(image.height * scale);
        ImageResize(&image, newWidth, newHeight);
    }
    
    currentTexture = LoadTextureFromImage(image);
    UnloadImage(image);
    
    if (currentTexture.id == 0) {
        ErrorClass::displayError("Failed to create texture from image: " + imagePath);
        hasImage = false;
        return;
    }
    
    hasImage = true;
    zoomLevel = 1.0f;
    panOffset = {0, 0};
    std::cout << "Image loaded successfully: " << imagePath << std::endl;
}

void CanvasClass::saveImage(const std::string& imagePath) {
    if (!hasImage || currentTexture.id == 0) {
        ErrorClass::displayError("No image to save");
        return;
    }
    
    Image image = LoadImageFromTexture(currentTexture);
    bool success = ExportImage(image, imagePath.c_str());
    UnloadImage(image);
    
    if (success) {
        std::cout << "Image saved successfully: " << imagePath << std::endl;
    } else {
        ErrorClass::displayError("Failed to save image: " + imagePath);
    }
}

void CanvasClass::update() {
    if (!hasImage) return;
    
    // Handle zoom with mouse wheel
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
        Vector2 mousePos = GetMousePosition();
        if (CheckCollisionPointRec(mousePos, canvasArea)) {
            zoomLevel += wheel * 0.1f;
            if (zoomLevel < 0.1f) zoomLevel = 0.1f;
            if (zoomLevel > 5.0f) zoomLevel = 5.0f;
        }
    }
    
    // Handle panning with middle mouse button or arrow keys
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        Vector2 mouseDelta = GetMouseDelta();
        panOffset.x += mouseDelta.x;
        panOffset.y += mouseDelta.y;
    }
    
    // Arrow key panning
    if (IsKeyDown(KEY_LEFT)) panOffset.x += 2.0f;
    if (IsKeyDown(KEY_RIGHT)) panOffset.x -= 2.0f;
    if (IsKeyDown(KEY_UP)) panOffset.y += 2.0f;
    if (IsKeyDown(KEY_DOWN)) panOffset.y -= 2.0f;
}

void CanvasClass::draw() {
    // Draw canvas background
    DrawRectangleRec(canvasArea, WHITE);
    DrawRectangleLinesEx(canvasArea, 1, DARKGRAY);
    
    if (!hasImage || currentTexture.id == 0) {
        // Draw placeholder text
        const char* text = "No image loaded. Click 'Load Image' to get started.";
        int textWidth = MeasureText(text, 20);
        int textX = canvasArea.x + (canvasArea.width - textWidth) / 2;
        int textY = canvasArea.y + canvasArea.height / 2 - 10;
        DrawText(text, textX, textY, 20, LIGHTGRAY);
        return;
    }
    
    // Calculate image position with zoom and pan
    float imageWidth = currentTexture.width * zoomLevel;
    float imageHeight = currentTexture.height * zoomLevel;
    
    float imageX = canvasArea.x + (canvasArea.width - imageWidth) / 2 + panOffset.x;
    float imageY = canvasArea.y + (canvasArea.height - imageHeight) / 2 + panOffset.y;
    
    Rectangle destRect = {imageX, imageY, imageWidth, imageHeight};
    Rectangle sourceRect = {0, 0, (float)currentTexture.width, (float)currentTexture.height};
    
    // Enable scissor test to clip image to canvas area
    BeginScissorMode(canvasArea.x, canvasArea.y, canvasArea.width, canvasArea.height);
    DrawTexturePro(currentTexture, sourceRect, destRect, Vector2{0, 0}, 0.0f, WHITE);
    EndScissorMode();
}

void CanvasClass::cleanup() {
    if (hasImage && currentTexture.id != 0) {
        UnloadTexture(currentTexture);
        currentTexture = Texture2D{0};
        hasImage = false;
    }
}

// Stubbed methods for future painting functionality
void CanvasClass::drawPixel(int x, int y, Color color) {
    // TODO: Implement pixel drawing
    std::cout << "Draw pixel at (" << x << ", " << y << ") - Not yet implemented" << std::endl;
}

void CanvasClass::drawLine(Vector2 start, Vector2 end, Color color, float thickness) {
    // TODO: Implement line drawing
    std::cout << "Draw line - Not yet implemented" << std::endl;
}

void CanvasClass::drawCircle(Vector2 center, float radius, Color color) {
    // TODO: Implement circle drawing
    std::cout << "Draw circle - Not yet implemented" << std::endl;
}

void CanvasClass::drawRectangle(Rectangle rect, Color color) {
    // TODO: Implement rectangle drawing
    std::cout << "Draw rectangle - Not yet implemented" << std::endl;
}