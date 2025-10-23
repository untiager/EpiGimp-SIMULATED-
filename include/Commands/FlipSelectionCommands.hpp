#ifndef FLIP_SELECTION_COMMANDS_HPP
#define FLIP_SELECTION_COMMANDS_HPP

#include "../Core/ICommand.hpp"
#include "raylib.h"
#include "../Core/RaylibWrappers.hpp"
#include <memory>

namespace EpiGimp {

// Forward declaration
class Canvas;

/**
 * @brief Base class for flip selection operations that can be undone/redone
 * 
 * This command captures the state of the active layer before a flip selection action
 * and can restore it for undo operations.
 */
class FlipSelectionCommand : public ICommand {
protected:
    Canvas* canvas_;                             // Target canvas
    size_t targetLayerIndex_;                    // Index of the layer that was modified
    std::unique_ptr<Image> beforeState_;         // Layer state before the flip
    std::unique_ptr<Image> afterState_;          // Layer state after the flip
    Rectangle selectionRect_;                    // The selection rectangle that was flipped
    std::string description_;                    // Description of the flip action
    
    /**
     * @brief Perform the actual flip operation on the image
     * @param image The image to flip
     */
    virtual void performFlip(Image& image) = 0;
    
public:
    /**
     * @brief Construct a new Flip Selection Command
     * 
     * @param canvas Target canvas
     * @param description Description of the flip action
     */
    explicit FlipSelectionCommand(Canvas* canvas, const std::string& description);
    
    /**
     * @brief Destroy the Flip Selection Command
     */
    ~FlipSelectionCommand() override = default;
    
    // Non-copyable
    FlipSelectionCommand(const FlipSelectionCommand&) = delete;
    FlipSelectionCommand& operator=(const FlipSelectionCommand&) = delete;
    
    // Movable
    FlipSelectionCommand(FlipSelectionCommand&&) = default;
    FlipSelectionCommand& operator=(FlipSelectionCommand&&) = default;
    
    /**
     * @brief Execute the flip selection command
     * @return true if execution was successful, false otherwise
     */
    bool execute() override;
    
    /**
     * @brief Undo the flip selection command
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
     * @brief Capture the current state before flipping
     * 
     * This should be called before any flip operations
     */
    void captureBeforeState();
    
    /**
     * @brief Capture the current state after flipping
     * 
     * This should be called after flip operations
     */
    void captureAfterState();
};

/**
 * @brief Command for vertical flip selection operations
 */
class FlipSelectionVerticalCommand : public FlipSelectionCommand {
protected:
    void performFlip(Image& image) override;
    
public:
    explicit FlipSelectionVerticalCommand(Canvas* canvas);
};

/**
 * @brief Command for horizontal flip selection operations
 */
class FlipSelectionHorizontalCommand : public FlipSelectionCommand {
protected:
    void performFlip(Image& image) override;
    
public:
    explicit FlipSelectionHorizontalCommand(Canvas* canvas);
};

/**
 * @brief Factory functions to create flip selection commands
 */
std::unique_ptr<FlipSelectionVerticalCommand> createFlipSelectionVerticalCommand(Canvas* canvas);
std::unique_ptr<FlipSelectionHorizontalCommand> createFlipSelectionHorizontalCommand(Canvas* canvas);

} // namespace EpiGimp

#endif // FLIP_SELECTION_COMMANDS_HPP