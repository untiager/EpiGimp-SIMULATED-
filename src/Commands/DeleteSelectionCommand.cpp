#include "../../include/Commands/DeleteSelectionCommand.hpp"
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

DeleteSelectionCommand::DeleteSelectionCommand(Canvas* canvas, const std::string& description)
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
    
    std::cout << "DeleteSelectionCommand: Created for layer " << targetLayerIndex_ 
              << " with selection (" << selectionRect_.x << "," << selectionRect_.y 
              << ") " << selectionRect_.width << "x" << selectionRect_.height << std::endl;
}

bool DeleteSelectionCommand::execute()
{
    // Check if we stored valid selection data during construction
    if (selectionRect_.width <= 0 || selectionRect_.height <= 0 || !canvas_) {
        std::cout << "DeleteSelectionCommand: Cannot execute - no valid selection data stored" << std::endl;
        return false;
    }
    
    std::cout << "DeleteSelectionCommand: Executing deletion" << std::endl;
    
    // Capture before state if not already captured
    if (!beforeState_) {
        captureBeforeState();
    }
    
    // Perform the deletion
    canvas_->deleteSelectionInternal();
    
    // Capture after state
    captureAfterState();
    
    std::cout << "DeleteSelectionCommand: Execution completed" << std::endl;
    return true;
}

bool DeleteSelectionCommand::undo()
{
    if (!beforeState_) {
        std::cout << "DeleteSelectionCommand: No before state captured, cannot undo" << std::endl;
        return false;
    }
    
    std::cout << "DeleteSelectionCommand: Performing undo..." << std::endl;
    
    // Get the target layer
    auto* layer = canvas_->getLayer(targetLayerIndex_);
    if (!layer || !layer->texture) {
        std::cout << "DeleteSelectionCommand: Cannot undo - layer or texture not available" << std::endl;
        return false;
    }
    
    // Restore the before state to the layer
    layer->texture->beginDrawing();
    
    // Clear the render texture first
    ClearBackground(Color{0, 0, 0, 0});
    
    // Load the before state as a texture and draw it
    Texture2D beforeTexture = LoadTextureFromImage(*beforeState_);
    DrawTexture(beforeTexture, 0, 0, WHITE);
    UnloadTexture(beforeTexture);
    
    layer->texture->endDrawing();
    
    std::cout << "DeleteSelectionCommand: Undo successful" << std::endl;
    return true;
}

std::string DeleteSelectionCommand::getDescription() const
{
    return description_;
}

bool DeleteSelectionCommand::canExecute() const
{
    return canvas_ && canvas_->hasSelection() && canvas_->hasDrawingTexture();
}

void DeleteSelectionCommand::captureBeforeState()
{
    if (targetLayerIndex_ >= static_cast<size_t>(canvas_->getLayerCount())) {
        std::cout << "DeleteSelectionCommand: Invalid layer index, cannot capture before state" << std::endl;
        return;
    }
    
    std::cout << "DeleteSelectionCommand: Capturing before state..." << std::endl;
    
    const auto* layer = canvas_->getLayer(targetLayerIndex_);
    if (!layer || !layer->texture) {
        std::cout << "DeleteSelectionCommand: Failed to capture before state - no layer or texture" << std::endl;
        return;
    }
    
    // Get the pixel data from the render texture
    Image layerImage = LoadImageFromTexture((**layer->texture).texture);
    
    // Flip the image vertically (OpenGL convention)
    ImageFlipVertical(&layerImage);
    
    // Store a copy of the image
    beforeState_ = std::make_unique<Image>(ImageCopy(layerImage));
    
    // Clean up the temporary image
    UnloadImage(layerImage);
    
    std::cout << "DeleteSelectionCommand: Before state captured (" 
              << beforeState_->width << "x" << beforeState_->height << ")" << std::endl;
}

void DeleteSelectionCommand::captureAfterState()
{
    if (targetLayerIndex_ >= static_cast<size_t>(canvas_->getLayerCount())) {
        std::cout << "DeleteSelectionCommand: Invalid layer index, cannot capture after state" << std::endl;
        return;
    }
    
    std::cout << "DeleteSelectionCommand: Capturing after state..." << std::endl;
    
    const auto* layer = canvas_->getLayer(targetLayerIndex_);
    if (!layer || !layer->texture) {
        std::cout << "DeleteSelectionCommand: Failed to capture after state - no layer or texture" << std::endl;
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
    
    std::cout << "DeleteSelectionCommand: After state captured (" 
              << afterState_->width << "x" << afterState_->height << ")" << std::endl;
}

std::unique_ptr<DeleteSelectionCommand> createDeleteSelectionCommand(Canvas* canvas, const std::string& description)
{
    return std::make_unique<DeleteSelectionCommand>(canvas, description);
}

} // namespace EpiGimp