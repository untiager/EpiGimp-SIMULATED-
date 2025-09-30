#ifndef DRAW_COMMAND_HPP
#define DRAW_COMMAND_HPP

#include "../Core/ICommand.hpp"
#include "raylib.h"
#include "../Core/RaylibWrappers.hpp"
#include <memory>
#include <vector>

namespace EpiGimp {

// Forward declaration
class Canvas;

/**
 * @brief Command for drawing operations that can be undone/redone
 * 
 * This command captures the state of the active layer before a drawing action
 * and can restore it for undo operations.
 */
class DrawCommand : public ICommand {
private:
    Canvas* canvas_;                              // Target canvas
    size_t targetLayerIndex_;                     // Index of the layer that was modified
    std::unique_ptr<Image> beforeState_;         // Layer state before the action
    std::unique_ptr<Image> afterState_;          // Layer state after the action
    std::string description_;                    // Description of the drawing action
    
public:
    /**
     * @brief Construct a new Draw Command
     * @param canvas Target canvas to operate on
     * @param description Description of the drawing action
     */
    DrawCommand(Canvas* canvas, const std::string& description);
    
    /**
     * @brief Destructor - properly clean up image resources
     */
    ~DrawCommand() override;
    
    /**
     * @brief Capture the current state before drawing begins
     * This should be called before the drawing operation starts
     */
    void captureBeforeState();
    
    /**
     * @brief Capture the current state after drawing completes
     * This should be called after the drawing operation finishes
     */
    void captureAfterState();
    
    // ICommand interface
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override { return description_; }
    bool canUndo() const override { return beforeState_ != nullptr; }
    
private:
    /**
     * @brief Copy the current active layer to an Image
     * @return Unique pointer to the copied image, or nullptr if failed
     */
    std::unique_ptr<Image> copyActiveLayerToImage() const;
    
    /**
     * @brief Restore the active layer from an Image
     * @param image The image to restore from
     * @return true if restoration was successful, false otherwise
     */
    bool restoreActiveLayerFromImage(const std::unique_ptr<Image>& image);
};

/**
 * @brief Factory function to create a draw command and capture initial state
 * @param canvas Target canvas
 * @param description Description of the drawing action
 * @return Unique pointer to the created command
 */
std::unique_ptr<DrawCommand> createDrawCommand(Canvas* canvas, const std::string& description);

} // namespace EpiGimp

#endif // DRAW_COMMAND_HPP