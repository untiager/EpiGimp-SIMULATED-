#include "../../include/Commands/DrawCommand.hpp"
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

DrawCommand::DrawCommand(Canvas* canvas, const std::string& description)
    : canvas_(canvas), description_(description) {
    if (!canvas_) {
        throw std::invalid_argument("Canvas cannot be null");
    }
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
    // Capture the current state of the drawing layer
    std::cout << "DrawCommand: Capturing before state..." << std::endl;
    beforeState_ = copyDrawingLayerToImage();
    if (!beforeState_) {
        std::cerr << "DrawCommand: Failed to capture before state" << std::endl;
    } else {
        std::cout << "DrawCommand: Before state captured (" << beforeState_->width << "x" << beforeState_->height << ")" << std::endl;
    }
}

void DrawCommand::captureAfterState() {
    // Capture the state after drawing
    std::cout << "DrawCommand: Capturing after state..." << std::endl;
    afterState_ = copyDrawingLayerToImage();
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
        return restoreDrawingLayerFromImage(afterState_);
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
    bool result = restoreDrawingLayerFromImage(beforeState_);
    if (result) {
        std::cout << "DrawCommand: Undo successful" << std::endl;
    } else {
        std::cout << "DrawCommand: Undo failed" << std::endl;
    }
    return result;
}

std::unique_ptr<Image> DrawCommand::copyDrawingLayerToImage() const {
    if (!canvas_) {
        return nullptr;
    }
    
    if (!canvas_->hasDrawingLayer()) {
        // If no drawing layer exists, we need to create a blank image with proper dimensions
        // This happens when capturing the "before" state of the first drawing action
        if (canvas_->hasImage()) {
            // Initialize the drawing layer to get proper dimensions
            const_cast<Canvas*>(canvas_)->initializeDrawingLayer();
            if (canvas_->hasDrawingLayer()) {
                // Now we can copy the blank drawing layer
                try {
                    Image copiedImage = canvas_->copyDrawingLayer();
                    return std::make_unique<Image>(copiedImage);
                }
                catch (const std::exception& e) {
                    std::cerr << "DrawCommand: Failed to copy newly initialized drawing layer: " << e.what() << std::endl;
                }
            }
        }
        return nullptr;
    }
    
    try {
        Image copiedImage = canvas_->copyDrawingLayer();
        return std::make_unique<Image>(copiedImage);
    }
    catch (const std::exception& e) {
        std::cerr << "DrawCommand: Failed to copy drawing layer: " << e.what() << std::endl;
        return nullptr;
    }
}

bool DrawCommand::restoreDrawingLayerFromImage(const std::unique_ptr<Image>& image) {
    if (!image || !canvas_) {
        return false;
    }
    
    return canvas_->restoreDrawingLayer(*image);
}

std::unique_ptr<DrawCommand> createDrawCommand(Canvas* canvas, const std::string& description) {
    auto command = std::make_unique<DrawCommand>(canvas, description);
    command->captureBeforeState();
    return command;
}

} // namespace EpiGimp