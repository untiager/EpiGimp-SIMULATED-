#include "../../include/Commands/LayerCommands.hpp"
#include <iostream>

namespace EpiGimp {

CreateLayerCommand::CreateLayerCommand(LayerManager* layerManager, const std::string& layerName)
    : layerManager_(layerManager)
    , layerName_(layerName)
    , createdLayerIndex_(0)
    , description_("Create Layer: " + layerName)
{
    if (!layerManager_)
        throw std::invalid_argument("LayerManager cannot be null");
}

bool CreateLayerCommand::execute()
{
    if (!layerManager_) return false;
    
    createdLayerIndex_ = layerManager_->createLayer(layerName_);
    std::cout << "CreateLayerCommand: Created layer '" << layerName_ << "' at index " << createdLayerIndex_ << std::endl;
    return true;
}

bool CreateLayerCommand::undo()
{
    if (!layerManager_) return false;
    
    bool result = layerManager_->deleteLayer(createdLayerIndex_);
    if (result)
        std::cout << "CreateLayerCommand: Undone - deleted layer at index " << createdLayerIndex_ << std::endl;
    return result;
}

DeleteLayerCommand::DeleteLayerCommand(LayerManager* layerManager, size_t layerIndex)
    : layerManager_(layerManager)
    , layerIndex_(layerIndex)
    , description_("Delete Layer")
{
    if (!layerManager_)
        throw std::invalid_argument("LayerManager cannot be null");
    
    const Layer* layer = layerManager_->getLayer(layerIndex);
    if (layer)
        description_ = "Delete Layer: " + layer->getName();
}

bool DeleteLayerCommand::execute()
{
    if (!layerManager_ || layerManager_->getLayerCount() <= 1)
        return false; // Can't delete the last layer
    
    // Store the layer before deleting (note: this is a simplified approach)
    // In a full implementation, we'd need to properly store and restore the layer
    bool result = layerManager_->deleteLayer(layerIndex_);
    if (result)
        std::cout << "DeleteLayerCommand: Deleted layer at index " << layerIndex_ << std::endl;
    return result;
}

bool DeleteLayerCommand::undo()
{
    // Note: This is a simplified implementation
    // A full implementation would need to restore the exact layer state
    if (!layerManager_) return false;
    
    // For now, just create a new layer at the same position
    // This is not a perfect undo but demonstrates the pattern
    std::string newName = "Restored Layer";
    size_t newIndex = layerManager_->createLayer(newName);
    
    std::cout << "DeleteLayerCommand: Undo approximated - created new layer at index " << newIndex << std::endl;
    return true;
}

MoveLayerCommand::MoveLayerCommand(LayerManager* layerManager, size_t fromIndex, size_t toIndex)
    : layerManager_(layerManager)
    , fromIndex_(fromIndex)
    , toIndex_(toIndex)
    , description_("Move Layer from " + std::to_string(fromIndex) + " to " + std::to_string(toIndex))
    {
    if (!layerManager_)
        throw std::invalid_argument("LayerManager cannot be null");
}

bool MoveLayerCommand::execute()
{
    if (!layerManager_) return false;
    
    bool result = layerManager_->moveLayer(fromIndex_, toIndex_);
    if (result)
        std::cout << "MoveLayerCommand: Moved layer from " << fromIndex_ << " to " << toIndex_ << std::endl;
    return result;
}

bool MoveLayerCommand::undo()
{
    if (!layerManager_) return false;
    
    bool result = layerManager_->moveLayer(toIndex_, fromIndex_);
    if (result)
        std::cout << "MoveLayerCommand: Undone - moved layer from " << toIndex_ << " back to " << fromIndex_ << std::endl;
    return result;
}

ToggleLayerVisibilityCommand::ToggleLayerVisibilityCommand(LayerManager* layerManager, size_t layerIndex)
    : layerManager_(layerManager)
    , layerIndex_(layerIndex)
    , previousVisibility_(true)
    , description_("Toggle Layer Visibility")
{
    if (!layerManager_)
        throw std::invalid_argument("LayerManager cannot be null");
    
    const Layer* layer = layerManager_->getLayer(layerIndex);
    if (layer) {
        previousVisibility_ = layer->isVisible();
        description_ = "Toggle Layer Visibility: " + layer->getName();
    }
}

bool ToggleLayerVisibilityCommand::execute()
{
    if (!layerManager_) return false;
    
    const Layer* layer = layerManager_->getLayer(layerIndex_);
    if (!layer) return false;
    
    bool newVisibility = !layer->isVisible();
    bool result = layerManager_->setLayerVisibility(layerIndex_, newVisibility);
    if (result)
        std::cout << "ToggleLayerVisibilityCommand: Set visibility to " << newVisibility << " for layer " << layerIndex_ << std::endl;
    return result;
}

bool ToggleLayerVisibilityCommand::undo()
{
    if (!layerManager_) return false;
    
    bool result = layerManager_->setLayerVisibility(layerIndex_, previousVisibility_);
    if (result)
        std::cout << "ToggleLayerVisibilityCommand: Restored visibility to " << previousVisibility_ << " for layer " << layerIndex_ << std::endl;
    return result;
}

SetLayerOpacityCommand::SetLayerOpacityCommand(LayerManager* layerManager, size_t layerIndex, float opacity)
    : layerManager_(layerManager)
    , layerIndex_(layerIndex)
    , newOpacity_(opacity)
    , previousOpacity_(1.0f)
    , description_("Set Layer Opacity")
{
    if (!layerManager_)
        throw std::invalid_argument("LayerManager cannot be null");
    
    const Layer* layer = layerManager_->getLayer(layerIndex);
    if (layer) {
        previousOpacity_ = layer->getOpacity();
        description_ = "Set Layer Opacity: " + layer->getName();
    }
}

bool SetLayerOpacityCommand::execute()
{
    if (!layerManager_) return false;
    
    bool result = layerManager_->setLayerOpacity(layerIndex_, newOpacity_);
    if (result)
        std::cout << "SetLayerOpacityCommand: Set opacity to " << newOpacity_ << " for layer " << layerIndex_ << std::endl;
    return result;
}

bool SetLayerOpacityCommand::undo()
{
    if (!layerManager_) return false;
    
    bool result = layerManager_->setLayerOpacity(layerIndex_, previousOpacity_);
    if (result)
        std::cout << "SetLayerOpacityCommand: Restored opacity to " << previousOpacity_ << " for layer " << layerIndex_ << std::endl;
    return result;
}

DuplicateLayerCommand::DuplicateLayerCommand(LayerManager* layerManager, size_t sourceLayerIndex)
    : layerManager_(layerManager)
    , sourceLayerIndex_(sourceLayerIndex)
    , createdLayerIndex_(0)
    , description_("Duplicate Layer")
{
    if (!layerManager_)
        throw std::invalid_argument("LayerManager cannot be null");
    
    const Layer* layer = layerManager_->getLayer(sourceLayerIndex);
    if (layer)
        description_ = "Duplicate Layer: " + layer->getName();
}

bool DuplicateLayerCommand::execute()
{
    if (!layerManager_) return false;
    
    bool result = layerManager_->duplicateLayer(sourceLayerIndex_);
    if (result) {
        createdLayerIndex_ = sourceLayerIndex_ + 1; // Duplicate is created after the source
        std::cout << "DuplicateLayerCommand: Duplicated layer " << sourceLayerIndex_ << " to " << createdLayerIndex_ << std::endl;
    }
    return result;
}

bool DuplicateLayerCommand::undo()
{
    if (!layerManager_) return false;
    
    bool result = layerManager_->deleteLayer(createdLayerIndex_);
    if (result)
        std::cout << "DuplicateLayerCommand: Undone - deleted duplicated layer at " << createdLayerIndex_ << std::endl;
    return result;
}

} // namespace EpiGimp