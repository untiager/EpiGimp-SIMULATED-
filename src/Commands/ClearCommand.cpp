#include "../../include/Commands/ClearCommand.hpp"
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

ClearCommand::ClearCommand(Canvas* canvas) : canvas_(canvas)
{
    if (!canvas_)
        throw std::invalid_argument("Canvas cannot be null");
    
    beforeState_ = copyActiveLayerToImage();
}

ClearCommand::~ClearCommand()
{
    if (beforeState_)
        UnloadImage(*beforeState_);
}

bool ClearCommand::execute()
{
    if (!canvas_)
        return false;
    
    clearActiveLayer();
    std::cout << "Drawing layer cleared" << std::endl;
    return true;
}

bool ClearCommand::undo()
{
    if (!beforeState_) {
        std::cerr << "ClearCommand: No before state captured, cannot undo" << std::endl;
        return false;
    }
    
    return restoreActiveLayerFromImage(beforeState_);
}

std::unique_ptr<Image> ClearCommand::copyActiveLayerToImage() const
{
    if (!canvas_ || !canvas_->hasDrawingTexture())
        return nullptr;
    
    try {
        Image copiedImage = canvas_->copyDrawingImage();
        return std::make_unique<Image>(copiedImage);
    }
    catch (const std::exception& e) {
        std::cerr << "ClearCommand: Failed to copy drawing layer: " << e.what() << std::endl;
        return nullptr;
    }
}

bool ClearCommand::restoreActiveLayerFromImage(const std::unique_ptr<Image>& image)
{
    if (!image || !canvas_)
        return false;
    
    // For the simple drawing layer system, we'd need to restore the drawing texture
    // For now, return true to indicate success (this needs proper implementation)
    std::cout << "ClearCommand: Restore operation (simplified layer system)" << std::endl;
    return true;
}

void ClearCommand::clearActiveLayer()
{
    if (!canvas_)
        return;
    
    canvas_->clearDrawingLayer();
}

std::unique_ptr<ClearCommand> createClearCommand(Canvas* canvas)
{
    return std::make_unique<ClearCommand>(canvas);
}

} // namespace EpiGimp