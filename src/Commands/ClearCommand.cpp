#include "../../include/Commands/ClearCommand.hpp"
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

ClearCommand::ClearCommand(Canvas* canvas) : canvas_(canvas) {
    if (!canvas_) {
        throw std::invalid_argument("Canvas cannot be null");
    }
    
    // Capture the current state before clearing
    beforeState_ = copyDrawingLayerToImage();
}

ClearCommand::~ClearCommand() {
    if (beforeState_) {
        UnloadImage(*beforeState_);
    }
}

bool ClearCommand::execute() {
    if (!canvas_) {
        return false;
    }
    
    clearDrawingLayer();
    std::cout << "Drawing layer cleared" << std::endl;
    return true;
}

bool ClearCommand::undo() {
    if (!beforeState_) {
        std::cerr << "ClearCommand: No before state captured, cannot undo" << std::endl;
        return false;
    }
    
    return restoreDrawingLayerFromImage(beforeState_);
}

std::unique_ptr<Image> ClearCommand::copyDrawingLayerToImage() const {
    if (!canvas_ || !canvas_->hasDrawingLayer()) {
        return nullptr;
    }
    
    try {
        Image copiedImage = canvas_->copyDrawingLayer();
        return std::make_unique<Image>(copiedImage);
    }
    catch (const std::exception& e) {
        std::cerr << "ClearCommand: Failed to copy drawing layer: " << e.what() << std::endl;
        return nullptr;
    }
}

bool ClearCommand::restoreDrawingLayerFromImage(const std::unique_ptr<Image>& image) {
    if (!image || !canvas_) {
        return false;
    }
    
    return canvas_->restoreDrawingLayer(*image);
}

void ClearCommand::clearDrawingLayer() {
    if (!canvas_) {
        return;
    }
    
    // Create a blank image with the same dimensions and restore it
    if (canvas_->hasImage()) {
        // Get the current drawing layer to know its size
        if (canvas_->hasDrawingLayer()) {
            Image currentLayer = canvas_->copyDrawingLayer();
            Image blankImage = GenImageColor(currentLayer.width, currentLayer.height, BLANK);
            canvas_->restoreDrawingLayer(blankImage);
            UnloadImage(blankImage);
            UnloadImage(currentLayer);
        }
    }
}

std::unique_ptr<ClearCommand> createClearCommand(Canvas* canvas) {
    return std::make_unique<ClearCommand>(canvas);
}

} // namespace EpiGimp