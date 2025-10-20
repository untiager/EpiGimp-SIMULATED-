#ifndef DELETE_SELECTION_COMMAND_HPP
#define DELETE_SELECTION_COMMAND_HPP

#include "../Core/ICommand.hpp"
#include "raylib.h"
#include "../Core/RaylibWrappers.hpp"
#include <memory>

namespace EpiGimp {

// Forward declaration
class Canvas;

/**
 * @brief Command for deleting selection operations that can be undone/redone
 * 
 * This command captures the state of the active layer before a delete selection action
 * and can restore it for undo operations.
 */
class DeleteSelectionCommand : public ICommand {
private:
    Canvas* canvas_;                             // Target canvas
    size_t targetLayerIndex_;                    // Index of the layer that was modified
    std::unique_ptr<Image> beforeState_;         // Layer state before the deletion
    std::unique_ptr<Image> afterState_;          // Layer state after the deletion
    Rectangle selectionRect_;                    // The selection rectangle that was deleted
    std::string description_;                    // Description of the delete action
    
public:
    /**
     * @brief Construct a new Delete Selection Command
     * 
     * @param canvas Target canvas
     * @param description Description of the delete action
     */
    explicit DeleteSelectionCommand(Canvas* canvas, const std::string& description = "Delete Selection");
    
    /**
     * @brief Destroy the Delete Selection Command
     */
    ~DeleteSelectionCommand() override = default;
    
    // Non-copyable
    DeleteSelectionCommand(const DeleteSelectionCommand&) = delete;
    DeleteSelectionCommand& operator=(const DeleteSelectionCommand&) = delete;
    
    // Movable
    DeleteSelectionCommand(DeleteSelectionCommand&&) = default;
    DeleteSelectionCommand& operator=(DeleteSelectionCommand&&) = default;
    
    /**
     * @brief Execute the delete selection command
     * @return true if execution was successful, false otherwise
     */
    bool execute() override;
    
    /**
     * @brief Undo the delete selection command
     * @return true if undo was successful, false otherwise
     */
    bool undo() override;
    
    /**
     * @brief Get description of the command
     * 
     * @return std::string Command description
     */
    std::string getDescription() const override;
    
    /**
     * @brief Check if the command can be executed
     * 
     * @return true If canvas has a selection and drawing texture
     * @return false Otherwise
     */
    bool canExecute() const;
    
    /**
     * @brief Capture the current state before deletion
     * 
     * This should be called before any deletion operations
     */
    void captureBeforeState();
    
    /**
     * @brief Capture the current state after deletion
     * 
     * This should be called after deletion operations
     */
    void captureAfterState();
};

/**
 * @brief Factory function to create a delete selection command
 * 
 * @param canvas Target canvas
 * @param description Command description
 * @return std::unique_ptr<DeleteSelectionCommand> New delete selection command
 */
std::unique_ptr<DeleteSelectionCommand> createDeleteSelectionCommand(Canvas* canvas, const std::string& description = "Delete Selection");

} // namespace EpiGimp

#endif // DELETE_SELECTION_COMMAND_HPP