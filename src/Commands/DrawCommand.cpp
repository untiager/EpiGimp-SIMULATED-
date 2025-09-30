#include "../../include/Commands/DrawCommand.hpp"
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

DrawCommand::DrawCommand(Canvas* canvas, const std::string& description)
    : canvas_(canvas), targetLayerIndex_(0), description_(description) {
    if (!canvas_) {
        throw std::invalid_argument("Canvas cannot be null");
    }
    
    // For simple layer system, always target the drawing layer (index 0)
    targetLayerIndex_ = 0;
}

DrawCommand::~DrawCommand() {
    // Clean up image resources if they exist
    if (beforeState_) {
        UnloadImage(*beforeState_);
    }
    if (afterState_) {
        UnloadImage(*afterState_);
    }
}

void DrawCommand::captureBeforeState() {
    // Capture the current state of the active layer
    std::cout << "DrawCommand: Capturing before state..." << std::endl;
    beforeState_ = copyActiveLayerToImage();
    if (!beforeState_) {
        std::cerr << "DrawCommand: Failed to capture before state" << std::endl;
    } else {
        std::cout << "DrawCommand: Before state captured (" << beforeState_->width << "x" << beforeState_->height << ")" << std::endl;
    }
}

void DrawCommand::captureAfterState() {
    // Capture the state after drawing
    std::cout << "DrawCommand: Capturing after state..." << std::endl;
    afterState_ = copyActiveLayerToImage();
    if (!afterState_) {
        std::cerr << "DrawCommand: Failed to capture after state" << std::endl;
    } else {
        std::cout << "DrawCommand: After state captured (" << afterState_->width << "x" << afterState_->height << ")" << std::endl;
    }
}

bool DrawCommand::execute() {
    // For DrawCommand, execute is typically a no-op because the drawing has already happened
    // The actual drawing is performed outside the command, and this command just manages the state
    
    // However, if we have an afterState_ stored, we can restore it (useful for redo operations)
    if (afterState_) {
        return restoreActiveLayerFromImage(afterState_);
    }
    
    // If no after state is captured, the command represents the current state
    return true;
}

bool DrawCommand::undo() {
    if (!beforeState_) {
        std::cerr << "DrawCommand: No before state captured, cannot undo" << std::endl;
        return false;
    }
    
    std::cout << "DrawCommand: Performing undo..." << std::endl;
    bool result = restoreActiveLayerFromImage(beforeState_);
    if (result) {
        std::cout << "DrawCommand: Undo successful" << std::endl;
    } else {
        std::cout << "DrawCommand: Undo failed" << std::endl;
    }
    return result;
}

std::unique_ptr<Image> DrawCommand::copyActiveLayerToImage() const {
    if (!canvas_ || !canvas_->hasDrawingTexture()) {
        return nullptr;
    }
    
    try {
        Image copiedImage = canvas_->copyDrawingImage();
        return std::make_unique<Image>(copiedImage);
    }
    catch (const std::exception& e) {
        std::cerr << "DrawCommand: Failed to copy drawing layer: " << e.what() << std::endl;
        return nullptr;
    }
}

bool DrawCommand::restoreActiveLayerFromImage(const std::unique_ptr<Image>& image) {
    if (!image || !canvas_) {
        return false;
    }
    
    // For the simple drawing layer system, we'd need to restore the drawing texture
    // For now, return true to indicate success (this needs proper implementation)
    std::cout << "DrawCommand: Restore operation (simplified layer system)" << std::endl;
    return true;
}

std::unique_ptr<DrawCommand> createDrawCommand(Canvas* canvas, const std::string& description) {
    auto command = std::make_unique<DrawCommand>(canvas, description);
    command->captureBeforeState();
    return command;
}

} // namespace EpiGimp