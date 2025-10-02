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
    
    Image testImage = LoadImageFromTexture(**currentTexture_);
    std::cout << "Canvas: Extracted test image: " << testImage.width << "x" << testImage.height << " format=" << testImage.format << std::endl;
    ExportImage(testImage, "/tmp/debug_loaded_texture.png");
    std::cout << "Canvas: Saved debug texture to /tmp/debug_loaded_texture.png" << std::endl;
    UnloadImage(testImage);
    
    // Update layer manager size and load image into background layer
    // Initialize drawing texture for the new image
    if (currentTexture_)
        initializeDrawingTexture();
    
    // IMPORTANT: Reset view transform to prevent coordinate issues
    resetViewTransform();
    std::cout << "Canvas: View transform reset to defaults" << std::endl;
    
    eventDispatcher_->emit<ImageLoadedEvent>(filePath);
    std::cout << "Image loaded successfully: " << filePath << std::endl;
}

void Canvas::createBlankCanvas(int width, int height, Color backgroundColor)
{
    Image blankImage = GenImageColor(width, height, backgroundColor);
    
    currentTexture_ = TextureResource::fromImage(blankImage);
    currentImagePath_ = ""; // No file path for blank canvas
    
    std::cout << "Canvas: Created blank canvas " << width << "x" << height << std::endl;
    
    UnloadImage(blankImage);
    
    if (currentTexture_)
        initializeDrawingTexture();
    
    resetViewTransform();
    std::cout << "Canvas: View transform reset to defaults" << std::endl;
    
    addNewDrawingLayer("Layer 1");
    
    eventDispatcher_->emit<ImageLoadedEvent>("blank_canvas");
    std::cout << "Blank canvas created successfully with initial layer" << std::endl;
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
    
    try {
        std::filesystem::path parentPath = std::filesystem::path(filePath).parent_path();
        if (!parentPath.empty() && !std::filesystem::exists(parentPath))
            std::filesystem::create_directories(parentPath);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to create directories: " << e.what() << std::endl;
        // Continue anyway - the save operation might still work
    }
    
    Image compositeImage;
    
    if (backgroundVisible_ && currentTexture_) {
        compositeImage = LoadImageFromTexture(**currentTexture_);
        
        // Blend all visible drawing layers on top
        for (const auto& layer : drawingLayers_) {
            if (layer.visible && layer.texture) {
                Image layerImage = LoadImageFromTexture((**layer.texture).texture);
                ImageFlipVertical(&layerImage); // Fix texture orientation
                
                // Simple blend the layer onto the composite
                for (int y = 0; y < compositeImage.height && y < layerImage.height; y++) {
                    for (int x = 0; x < compositeImage.width && x < layerImage.width; x++) {
                        Color layerPixel = GetImageColor(layerImage, x, y);
                        if (layerPixel.a > 0) { // If layer pixel is not transparent
                            ImageDrawPixel(&compositeImage, x, y, layerPixel);
                        }
                    }
                }
                
                UnloadImage(layerImage);
            }
        }
    } else if (!drawingLayers_.empty()) {
        // Only drawing layers are visible - create composite from layers
        bool hasVisibleLayer = false;
        for (const auto& layer : drawingLayers_) {
            if (layer.visible && layer.texture) {
                if (!hasVisibleLayer) {
                    // First visible layer becomes the base
                    compositeImage = LoadImageFromTexture((**layer.texture).texture);
                    ImageFlipVertical(&compositeImage);
                    hasVisibleLayer = true;
                } else {
                    // Blend subsequent layers
                    Image layerImage = LoadImageFromTexture((**layer.texture).texture);
                    ImageFlipVertical(&layerImage);
                    
                    for (int y = 0; y < compositeImage.height && y < layerImage.height; y++) {
                        for (int x = 0; x < compositeImage.width && x < layerImage.width; x++) {
                            Color layerPixel = GetImageColor(layerImage, x, y);
                            if (layerPixel.a > 0) {
                                ImageDrawPixel(&compositeImage, x, y, layerPixel);
                            }
                        }
                    }
                    
                    UnloadImage(layerImage);
                }
            }
        }
        
        if (!hasVisibleLayer) {
            eventDispatcher_->emit<ErrorEvent>("No visible layers to save");
            return false;
        }
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
    
    if (backgroundVisible_ && currentTexture_)
        DrawTexture(**currentTexture_, static_cast<int>(imageDestRect.x), static_cast<int>(imageDestRect.y), WHITE);
    
    // Draw layers in reverse order so that layer 0 (top of the list) appears on top visually
    for (int i = static_cast<int>(drawingLayers_.size()) - 1; i >= 0; --i) {
        const auto& layer = drawingLayers_[i];
        if (layer.visible && layer.texture) {
            const Texture2D& layerTex = (**layer.texture).texture;
            
            // Source rectangle with negative height to flip Y-axis for RenderTexture coordinate system
            Rectangle sourceRect = {0, 0, static_cast<float>(layerTex.width), static_cast<float>(-layerTex.height)};
            
            DrawTexturePro(layerTex, sourceRect, imageDestRect, Vector2{0, 0}, 0.0f, WHITE);
        }
    }
}

} // namespace EpiGimp