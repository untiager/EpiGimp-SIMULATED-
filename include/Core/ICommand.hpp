#ifndef ICOMMAND_HPP
#define ICOMMAND_HPP

#include <memory>
#include <string>

namespace EpiGimp {

/**
 * @brief Abstract base class for implementing the Command pattern
 * 
 * This interface defines commands that can be executed and undone,
 * enabling undo/redo functionality for various operations.
 */
class ICommand {
public:
    virtual ~ICommand() = default;
    
    /**
     * @brief Execute the command
     * @return true if execution was successful, false otherwise
     */
    virtual bool execute() = 0;
    
    /**
     * @brief Undo the command (reverse its effects)
     * @return true if undo was successful, false otherwise
     */
    virtual bool undo() = 0;
    
    /**
     * @brief Get a description of this command for debugging/UI
     * @return String description of the command
     */
    virtual std::string getDescription() const = 0;
    
    /**
     * @brief Check if this command can be undone
     * @return true if the command supports undo, false otherwise
     */
    virtual bool canUndo() const { return true; }
};

using CommandPtr = std::unique_ptr<ICommand>;

} // namespace EpiGimp

#endif // ICOMMAND_HPP