#ifndef LAYERCOMMANDS_HPP
#define LAYERCOMMANDS_HPP

#include "../Core/ICommand.hpp"
#include "../Core/LayerManager.hpp"
#include <string>
#include <memory>

namespace EpiGimp {

/**
 * @brief Command for creating a new layer
 */
class CreateLayerCommand : public ICommand {
private:
    LayerManager* layerManager_;
    std::string layerName_;
    size_t createdLayerIndex_;
    std::string description_;

public:
    CreateLayerCommand(LayerManager* layerManager, const std::string& layerName);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override { return description_; }
    bool canUndo() const override { return true; }
};

/**
 * @brief Command for deleting a layer
 */
class DeleteLayerCommand : public ICommand {
private:
    LayerManager* layerManager_;
    size_t layerIndex_;
    std::unique_ptr<Layer> deletedLayer_;  // Store the deleted layer for undo
    std::string description_;

public:
    DeleteLayerCommand(LayerManager* layerManager, size_t layerIndex);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override { return description_; }
    bool canUndo() const override { return deletedLayer_ != nullptr; }
};

/**
 * @brief Command for moving/reordering layers
 */
class MoveLayerCommand : public ICommand {
private:
    LayerManager* layerManager_;
    size_t fromIndex_;
    size_t toIndex_;
    std::string description_;

public:
    MoveLayerCommand(LayerManager* layerManager, size_t fromIndex, size_t toIndex);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override { return description_; }
    bool canUndo() const override { return true; }
};

/**
 * @brief Command for changing layer visibility
 */
class ToggleLayerVisibilityCommand : public ICommand {
private:
    LayerManager* layerManager_;
    size_t layerIndex_;
    bool previousVisibility_;
    std::string description_;

public:
    ToggleLayerVisibilityCommand(LayerManager* layerManager, size_t layerIndex);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override { return description_; }
    bool canUndo() const override { return true; }
};

/**
 * @brief Command for changing layer opacity
 */
class SetLayerOpacityCommand : public ICommand {
private:
    LayerManager* layerManager_;
    size_t layerIndex_;
    float newOpacity_;
    float previousOpacity_;
    std::string description_;

public:
    SetLayerOpacityCommand(LayerManager* layerManager, size_t layerIndex, float opacity);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override { return description_; }
    bool canUndo() const override { return true; }
};

/**
 * @brief Command for duplicating a layer
 */
class DuplicateLayerCommand : public ICommand {
private:
    LayerManager* layerManager_;
    size_t sourceLayerIndex_;
    size_t createdLayerIndex_;
    std::string description_;

public:
    DuplicateLayerCommand(LayerManager* layerManager, size_t sourceLayerIndex);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override { return description_; }
    bool canUndo() const override { return true; }
};

} // namespace EpiGimp

#endif // LAYERCOMMANDS_HPP