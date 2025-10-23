#include "../../include/Commands/FlipSelectionCommands.hpp"
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

// Base FlipSelectionCommand implementation
FlipSelectionCommand::FlipSelectionCommand(Canvas* canvas, const std::string& description)
    : canvas_(canvas), targetLayerIndex_(0), selectionRect_{0, 0, 0, 0}, description_(description)
{
    if (!canvas_) {
        throw std::invalid_argument("Canvas cannot be null");
    }
    
    // Store current layer index and selection rectangle
    targetLayerIndex_ = canvas_->getSelectedLayerIndex();
    if (canvas_->hasSelection()) {
        selectionRect_ = canvas_->getSelectionRect();
    }
    
    std::cout << "FlipSelectionCommand: Created for layer " << targetLayerIndex_ 
              << " with selection (" << selectionRect_.x << "," << selectionRect_.y 
              << ") " << selectionRect_.width << "x" << selectionRect_.height << std::endl;
}

bool FlipSelectionCommand::execute()
{
    // Check if we stored valid selection data during construction
    if (selectionRect_.width <= 0 || selectionRect_.height <= 0 || !canvas_) {
        std::cout << "FlipSelectionCommand: Cannot execute - no valid selection data stored" << std::endl;
        return false;
    }
    
    std::cout << "FlipSelectionCommand: Executing flip" << std::endl;
    
    // Capture before state if not already captured
    if (!beforeState_) {
        captureBeforeState();
    }
    
    // Get the target layer
    auto* layer = canvas_->getLayer(targetLayerIndex_);
    if (!layer || !layer->texture) {
        std::cout << "FlipSelectionCommand: Cannot execute - layer or texture not available" << std::endl;
        return false;
    }
    
    // Extract current layer content as image
    Image layerImage = LoadImageFromTexture((**layer->texture).texture);
    
    // Flip the image vertically to match the screen coordinate system
    // (OpenGL textures are upside-down relative to screen coordinates)
    ImageFlipVertical(&layerImage);
    
    // Now we can use the selection rectangle directly - no transformation needed
    // The coordinates are already in the correct space after the flip
    Rectangle extractRect = {
        selectionRect_.x,
        selectionRect_.y,
        selectionRect_.width,
        selectionRect_.height
    };
    
    std::cout << "FlipSelectionCommand: Extracting from (" << extractRect.x << "," << extractRect.y 
              << ") " << extractRect.width << "x" << extractRect.height << std::endl;
    
    // Validate extraction rectangle
    if (extractRect.x < 0 || extractRect.y < 0 || 
        extractRect.x + extractRect.width > layerImage.width ||
        extractRect.y + extractRect.height > layerImage.height) {
        std::cout << "FlipSelectionCommand: Invalid extraction coordinates" << std::endl;
        UnloadImage(layerImage);
        return false;
    }
    
    // Extract the selection area
    Image selectionImage = ImageFromImage(layerImage, extractRect);
    
    // Check if we extracted any content
    Color* pixels = LoadImageColors(selectionImage);
    bool hasContent = false;
    for (int i = 0; i < selectionImage.width * selectionImage.height; i++) {
        if (pixels[i].a > 0) {
            hasContent = true;
            break;
        }
    }
    UnloadImageColors(pixels);
    
    if (!hasContent) {
        std::cout << "FlipSelectionCommand: No content found in selection area" << std::endl;
        UnloadImage(selectionImage);
        UnloadImage(layerImage);
        return false;
    }
    
    // Perform the flip operation (implemented by derived classes)
    performFlip(selectionImage);
    
    // Now we need to update the layer texture directly
    // First, clear the selection area (like deleteSelectionInternal does)
    layer->texture->beginDrawing();
    
    // Use scissor test to limit clearing to the selection area
    BeginScissorMode((int)selectionRect_.x, (int)selectionRect_.y, 
                     (int)selectionRect_.width, (int)selectionRect_.height);
    
    // Clear the background to transparent in the scissored area
    ClearBackground(Color{0, 0, 0, 0});
    
    EndScissorMode();
    
    // Now draw the flipped selection back to the texture at the same position
    // Convert the flipped image back to a texture and draw it
    Texture2D flippedTexture = LoadTextureFromImage(selectionImage);
    DrawTexture(flippedTexture, (int)selectionRect_.x, (int)selectionRect_.y, WHITE);
    UnloadTexture(flippedTexture);
    
    layer->texture->endDrawing();
    
    // Clean up
    UnloadImage(selectionImage);
    UnloadImage(layerImage);
    
    // Capture after state
    captureAfterState();
    
    std::cout << "FlipSelectionCommand: Execution completed" << std::endl;
    return true;
}

bool FlipSelectionCommand::undo()
{
    if (!beforeState_) {
        std::cout << "FlipSelectionCommand: No before state captured, cannot undo" << std::endl;
        return false;
    }
    
    std::cout << "FlipSelectionCommand: Performing undo..." << std::endl;
    
    // Get the target layer
    auto* layer = canvas_->getLayer(targetLayerIndex_);
    if (!layer || !layer->texture) {
        std::cout << "FlipSelectionCommand: Cannot undo - layer or texture not available" << std::endl;
        return false;
    }
    
    // Restore the before state to the layer (same as DeleteSelectionCommand)
    layer->texture->beginDrawing();
    
    // Clear the render texture first
    ClearBackground(Color{0, 0, 0, 0});
    
    // Load the before state as a texture and draw it
    Texture2D beforeTexture = LoadTextureFromImage(*beforeState_);
    DrawTexture(beforeTexture, 0, 0, WHITE);
    UnloadTexture(beforeTexture);
    
    layer->texture->endDrawing();
    
    std::cout << "FlipSelectionCommand: Undo successful" << std::endl;
    return true;
}

std::string FlipSelectionCommand::getDescription() const
{
    return description_;
}

bool FlipSelectionCommand::canExecute() const
{
    return canvas_ && canvas_->hasSelection() && canvas_->hasDrawingTexture();
}

void FlipSelectionCommand::captureBeforeState()
{
    if (targetLayerIndex_ >= static_cast<size_t>(canvas_->getLayerCount())) {
        std::cout << "FlipSelectionCommand: Invalid layer index, cannot capture before state" << std::endl;
        return;
    }
    
    std::cout << "FlipSelectionCommand: Capturing before state..." << std::endl;
    
    const auto* layer = canvas_->getLayer(targetLayerIndex_);
    if (!layer || !layer->texture) {
        std::cout << "FlipSelectionCommand: Failed to capture before state - no layer or texture" << std::endl;
        return;
    }
    
    // Get the pixel data from the render texture
    Image layerImage = LoadImageFromTexture((**layer->texture).texture);
    
    // Flip the image vertically (OpenGL convention) - same as DeleteSelectionCommand
    ImageFlipVertical(&layerImage);
    
    // Store a copy of the image
    beforeState_ = std::make_unique<Image>(ImageCopy(layerImage));
    
    // Clean up the temporary image
    UnloadImage(layerImage);
    
    std::cout << "FlipSelectionCommand: Before state captured (" 
              << beforeState_->width << "x" << beforeState_->height << ")" << std::endl;
}

void FlipSelectionCommand::captureAfterState()
{
    if (targetLayerIndex_ >= static_cast<size_t>(canvas_->getLayerCount())) {
        std::cout << "FlipSelectionCommand: Invalid layer index, cannot capture after state" << std::endl;
        return;
    }
    
    std::cout << "FlipSelectionCommand: Capturing after state..." << std::endl;
    
    const auto* layer = canvas_->getLayer(targetLayerIndex_);
    if (!layer || !layer->texture) {
        std::cout << "FlipSelectionCommand: Failed to capture after state - no layer or texture" << std::endl;
        return;
    }
    
    // Get the pixel data from the render texture
    Image layerImage = LoadImageFromTexture((**layer->texture).texture);
    
    // Flip the image vertically (OpenGL convention)
    ImageFlipVertical(&layerImage);
    
    // Store a copy of the image
    afterState_ = std::make_unique<Image>(ImageCopy(layerImage));
    
    // Clean up the temporary image
    UnloadImage(layerImage);
    
    std::cout << "FlipSelectionCommand: After state captured (" 
              << afterState_->width << "x" << afterState_->height << ")" << std::endl;
}

// FlipSelectionVerticalCommand implementation
FlipSelectionVerticalCommand::FlipSelectionVerticalCommand(Canvas* canvas)
    : FlipSelectionCommand(canvas, "Flip Selection Vertically")
{
}

void FlipSelectionVerticalCommand::performFlip(Image& image)
{
    ImageFlipVertical(&image);
}

// FlipSelectionHorizontalCommand implementation
FlipSelectionHorizontalCommand::FlipSelectionHorizontalCommand(Canvas* canvas)
    : FlipSelectionCommand(canvas, "Flip Selection Horizontally")
{
}

void FlipSelectionHorizontalCommand::performFlip(Image& image)
{
    ImageFlipHorizontal(&image);
}

// Factory functions
std::unique_ptr<FlipSelectionVerticalCommand> createFlipSelectionVerticalCommand(Canvas* canvas)
{
    return std::make_unique<FlipSelectionVerticalCommand>(canvas);
}

std::unique_ptr<FlipSelectionHorizontalCommand> createFlipSelectionHorizontalCommand(Canvas* canvas)
{
    return std::make_unique<FlipSelectionHorizontalCommand>(canvas);
}

}  // namespace EpiGimp