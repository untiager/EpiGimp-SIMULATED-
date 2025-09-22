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
    resetViewTransform();
    
    // Initialize drawing layer for the new image
    initializeDrawingLayer();
    
    eventDispatcher_->emit<ImageLoadedEvent>(filePath);
    std::cout << "Image loaded successfully: " << filePath << std::endl;
}

bool Canvas::saveImage(const std::string& filePath)
{
    if (!hasImage()) {
        eventDispatcher_->emit<ErrorEvent>("No image to save");
        return false;
    }
    
    std::filesystem::create_directories(std::filesystem::path(filePath).parent_path());
    
    // If we have drawings, composite them with the original image
    if (drawingLayer_ && drawingLayer_->isValid()) {
        // Get original image
        auto originalImage = ImageResource::fromTexture(**currentTexture_);
        if (!originalImage) {
            eventDispatcher_->emit<ErrorEvent>("Failed to create image from texture");
            return false;
        }
        
        // Get drawing layer as image (this handles the Y-flip automatically)
        auto drawingImage = ImageResource::fromTexture((**drawingLayer_).texture);
        if (!drawingImage) {
            eventDispatcher_->emit<ErrorEvent>("Failed to create drawing image");
            return false;
        }
        
        // Flip the drawing image vertically to correct the render texture flip
        ImageFlipVertical(drawingImage->getMutable());
        
        // Create a new image and composite them
        Image composite = GenImageColor(originalImage->get()->width, originalImage->get()->height, WHITE);
        
        // Draw original image onto composite
        ImageDraw(&composite, *originalImage->get(), 
                 Rectangle{0, 0, static_cast<float>(originalImage->get()->width), static_cast<float>(originalImage->get()->height)},
                 Rectangle{0, 0, static_cast<float>(composite.width), static_cast<float>(composite.height)}, WHITE);
        
        // Draw the drawing layer on top
        ImageDraw(&composite, *drawingImage->get(),
                 Rectangle{0, 0, static_cast<float>(drawingImage->get()->width), static_cast<float>(drawingImage->get()->height)},
                 Rectangle{0, 0, static_cast<float>(composite.width), static_cast<float>(composite.height)}, WHITE);
        
        // Create ImageResource from the composite
        ImageResource compositeRes(composite);
        
        std::string actualPath;
        const bool success = compositeRes.exportToFile(filePath, actualPath);
        
        // Inform user if filename was auto-corrected
        if (success && actualPath != filePath) {
            std::cout << "Note: File extension was auto-corrected to: " << actualPath << std::endl;
        }
        
        eventDispatcher_->emit<ImageSavedEvent>(actualPath, success);
        
        if (success) {
            std::cout << "Image saved successfully with drawings: " << actualPath << std::endl;
        } else {
            eventDispatcher_->emit<ErrorEvent>("Failed to save image: " + filePath);
        }
        
        return success;
    }
    
    // No drawings, save original image
    auto imageRes = ImageResource::fromTexture(**currentTexture_);
    if (!imageRes) {
        eventDispatcher_->emit<ErrorEvent>("Failed to create image from texture");
        return false;
    }
    
    std::string actualPath;
    const bool success = imageRes->exportToFile(filePath, actualPath);
    
    // Inform user if filename was auto-corrected
    if (success && actualPath != filePath) {
        std::cout << "Note: File extension was auto-corrected to: " << actualPath << std::endl;
    }
    
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
    if (!imageRes) {
        return std::nullopt;
    }
    
    // Resize image if too large for canvas
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
    
    DrawTexturePro(**currentTexture_, 
                  Rectangle{0, 0, static_cast<float>((*currentTexture_)->width), 
                           static_cast<float>((*currentTexture_)->height)},
                  imageDestRect, Vector2{0, 0}, 0, WHITE);
    
    // Draw the drawing layer on top of the image
    if (drawingLayer_ && drawingLayer_->isValid()) {
        DrawTexturePro((**drawingLayer_).texture,
                      Rectangle{0, 0, static_cast<float>((**drawingLayer_).texture.width),
                               -static_cast<float>((**drawingLayer_).texture.height)}, // Negative height to flip Y
                      imageDestRect, Vector2{0, 0}, 0, WHITE);
    }
}

} // namespace EpiGimp