#ifndef HISTORY_MANAGER_HPP
#define HISTORY_MANAGER_HPP

#include <stack>
#include <memory>
#include "ICommand.hpp"

namespace EpiGimp {

/**
 * @brief Manages command history for undo/redo functionality
 * 
 * This class maintains two stacks: one for undo operations and one for redo operations.
 * It follows the standard undo/redo behavior found in text editors and graphics applications.
 */
class HistoryManager {
private:
    std::stack<CommandPtr> undoStack_;
    std::stack<CommandPtr> redoStack_;
    size_t maxHistorySize_;
    
public:
    /**
     * @brief Construct a new History Manager
     * @param maxHistorySize Maximum number of commands to keep in history (default: 50)
     */
    explicit HistoryManager(size_t maxHistorySize = 50);
    
    /**
     * @brief Execute a command and add it to the undo stack
     * @param command The command to execute
     * @return true if command was executed successfully, false otherwise
     */
    bool executeCommand(CommandPtr command);
    
    /**
     * @brief Undo the last executed command
     * @return true if undo was successful, false if no commands to undo
     */
    bool undo();
    
    /**
     * @brief Redo the last undone command
     * @return true if redo was successful, false if no commands to redo
     */
    bool redo();
    
    /**
     * @brief Check if there are commands available to undo
     * @return true if undo is possible, false otherwise
     */
    bool canUndo() const;
    
    /**
     * @brief Check if there are commands available to redo
     * @return true if redo is possible, false otherwise
     */
    bool canRedo() const;
    
    /**
     * @brief Clear all command history
     */
    void clearHistory();
    
    /**
     * @brief Get the number of commands in the undo stack
     * @return Number of undoable commands
     */
    size_t getUndoCount() const;
    
    /**
     * @brief Get the number of commands in the redo stack
     * @return Number of redoable commands
     */
    size_t getRedoCount() const;
    
    /**
     * @brief Get description of the next command that would be undone
     * @return Description string, or empty if no commands to undo
     */
    std::string getNextUndoDescription() const;
    
    /**
     * @brief Get description of the next command that would be redone
     * @return Description string, or empty if no commands to redo
     */
    std::string getNextRedoDescription() const;

private:
    /**
     * @brief Ensure the undo stack doesn't exceed maximum size
     */
    void enforceMaxSize();
};

} // namespace EpiGimp

#endif // HISTORY_MANAGER_HPP