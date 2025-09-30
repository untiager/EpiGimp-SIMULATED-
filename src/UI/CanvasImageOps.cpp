//Canvas image handling functionality
#include "../../include/UI/Canvas.hpp"
#include <iostream>
#include <algorithm>
#include <filesystem>

namespace EpiGimp {

void Canvas::loadImage(const std::string& filePath)
{
    auto texture = createTextureFromFile(filePath);
    if (!texture) {
        eventDispatcher_->emit<ErrorEvent>("Failed to load image: " + filePath);
        return;
    }
    
    currentTexture_ = std::move(texture);
    currentImagePath_ = filePath;
    
    std::cout << "Canvas: Loaded texture " << (*currentTexture_)->width << "x" << (*currentTexture_)->height << std::endl;
    
    // TEMP: Save the loaded texture to verify it contains data
    Image testImage = LoadImageFromTexture(**currentTexture_);
    std::cout << "Canvas: Extracted test image: " << testImage.width << "x" << testImage.height << " format=" << testImage.format << std::endl;
    ExportImage(testImage, "/tmp/debug_loaded_texture.png");
    std::cout << "Canvas: Saved debug texture to /tmp/debug_loaded_texture.png" << std::endl;
    UnloadImage(testImage);
    
    // Update layer manager size and load image into background layer
    // Initialize drawing texture for the new image
    if (currentTexture_) {
        initializeDrawingTexture();
    }
    
    // IMPORTANT: Reset view transform to prevent coordinate issues
    resetViewTransform();
    std::cout << "Canvas: View transform reset to defaults" << std::endl;
    
    eventDispatcher_->emit<ImageLoadedEvent>(filePath);
    std::cout << "Image loaded successfully: " << filePath << std::endl;
}

bool Canvas::saveImage(const std::string& filePath)
{
    if (!hasImage()) {
        eventDispatcher_->emit<ErrorEvent>("No image to save");
        return false;
    }
    
    if (filePath.empty()) {
        eventDispatcher_->emit<ErrorEvent>("Invalid file path");
        return false;
    }
    
    // Create parent directories only if they don't exist and path is valid
    try {
        std::filesystem::path parentPath = std::filesystem::path(filePath).parent_path();
        if (!parentPath.empty() && !std::filesystem::exists(parentPath)) {
            std::filesystem::create_directories(parentPath);
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to create directories: " << e.what() << std::endl;
        // Continue anyway - the save operation might still work
    }
    
    // Create a composite image combining background and drawing layers
    Image compositeImage;
    
    if (backgroundVisible_ && currentTexture_) {
        // Start with the background image
        compositeImage = LoadImageFromTexture(**currentTexture_);
        
        // If drawing layer exists and is visible, blend it on top
        if (drawingVisible_ && hasDrawingTexture()) {
            Image drawingImage = LoadImageFromTexture((**drawingTexture_).texture);
            ImageFlipVertical(&drawingImage); // Fix texture orientation
            
            // Simple blend the drawing onto the background
            for (int y = 0; y < compositeImage.height && y < drawingImage.height; y++) {
                for (int x = 0; x < compositeImage.width && x < drawingImage.width; x++) {
                    Color drawingPixel = GetImageColor(drawingImage, x, y);
                    if (drawingPixel.a > 0) { // If drawing pixel is not transparent
                        ImageDrawPixel(&compositeImage, x, y, drawingPixel);
                    }
                }
            }
            
            UnloadImage(drawingImage);
        }
    } else if (drawingVisible_ && hasDrawingTexture()) {
        // Only drawing layer is visible
        compositeImage = LoadImageFromTexture((**drawingTexture_).texture);
        ImageFlipVertical(&compositeImage);
    } else {
        eventDispatcher_->emit<ErrorEvent>("No visible layers to save");
        return false;
    }
    
    ImageResource compositeRes(compositeImage);
    
    std::string actualPath;
    const bool success = compositeRes.exportToFile(filePath, actualPath);
    
    if (success && actualPath != filePath)
        std::cout << "Note: File extension was auto-corrected to: " << actualPath << std::endl;
    
    eventDispatcher_->emit<ImageSavedEvent>(actualPath, success);
    
    if (success) {
        std::cout << "Image saved successfully: " << actualPath << std::endl;
    } else {
        eventDispatcher_->emit<ErrorEvent>("Failed to save image: " + filePath);
    }
    
    return success;
}

std::optional<TextureResource> Canvas::createTextureFromFile(const std::string& filePath)
{
    auto imageRes = ImageResource::fromFile(filePath);
    if (!imageRes)
        return std::nullopt;
    
    const auto maxWidth = static_cast<int>(bounds_.width);
    const auto maxHeight = static_cast<int>(bounds_.height);
    
    if (imageRes->get()->width > maxWidth || imageRes->get()->height > maxHeight) {
        const float widthRatio = static_cast<float>(maxWidth) / imageRes->get()->width;
        const float heightRatio = static_cast<float>(maxHeight) / imageRes->get()->height;
        const float scale = std::min(widthRatio, heightRatio);
        
        const int newWidth = static_cast<int>(imageRes->get()->width * scale);
        const int newHeight = static_cast<int>(imageRes->get()->height * scale);
        
        imageRes->resize(newWidth, newHeight);
        std::cout << "Image resized to: " << newWidth << "x" << newHeight << std::endl;
    }
    
    return TextureResource::fromImage(*imageRes->get());
}

void Canvas::drawImage() const
{
    const Rectangle imageDestRect = calculateImageDestRect();
    
    static int debugCount = 0;
    debugCount++;
    
    if (debugCount % 60 == 0) {
        std::cout << "Drawing - zoom:" << zoomLevel_ << " pan:(" << panOffset_.x << "," << panOffset_.y << ")" 
                  << " imageRect:(" << imageDestRect.x << "," << imageDestRect.y << "," 
                  << imageDestRect.width << "," << imageDestRect.height << ")" << std::endl;
    }
    
    // Draw the background layer if visible
    if (backgroundVisible_ && currentTexture_) {
        DrawTexture(**currentTexture_, static_cast<int>(imageDestRect.x), static_cast<int>(imageDestRect.y), WHITE);
    }
    
    // Draw the drawing layer on top if visible
    if (drawingVisible_ && hasDrawingTexture()) {
        DrawTexture((**drawingTexture_).texture, static_cast<int>(imageDestRect.x), static_cast<int>(imageDestRect.y), WHITE);
    }
}

} // namespace EpiGimp