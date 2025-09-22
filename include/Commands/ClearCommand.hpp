#ifndef CLEAR_COMMAND_HPP
#define CLEAR_COMMAND_HPP

#include "../Core/ICommand.hpp"
#include "raylib.h"
#include <memory>

namespace EpiGimp {

// Forward declaration
class Canvas;

/**
 * @brief Command for clearing the drawing layer
 * 
 * This command captures the current drawing layer state and can restore it for undo.
 */
class ClearCommand : public ICommand {
private:
    Canvas* canvas_;
    std::unique_ptr<Image> beforeState_;
    
public:
    /**
     * @brief Construct a new Clear Command
     * @param canvas Target canvas to clear
     */
    explicit ClearCommand(Canvas* canvas);
    
    /**
     * @brief Destructor - clean up image resources
     */
    ~ClearCommand() override;
    
    // ICommand interface
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override { return "Clear drawing layer"; }
    bool canUndo() const override { return beforeState_ != nullptr; }

private:
    std::unique_ptr<Image> copyDrawingLayerToImage() const;
    bool restoreDrawingLayerFromImage(const std::unique_ptr<Image>& image);
    void clearDrawingLayer();
};

/**
 * @brief Factory function to create a clear command
 * @param canvas Target canvas
 * @return Unique pointer to the created command
 */
std::unique_ptr<ClearCommand> createClearCommand(Canvas* canvas);

} // namespace EpiGimp

#endif // CLEAR_COMMAND_HPP