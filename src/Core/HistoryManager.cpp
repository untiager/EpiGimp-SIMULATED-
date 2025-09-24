#include "../../include/Core/HistoryManager.hpp"
#include <iostream>

namespace EpiGimp {

HistoryManager::HistoryManager(size_t maxHistorySize) 
    : maxHistorySize_(maxHistorySize) {
}

bool HistoryManager::executeCommand(CommandPtr command)
{
    if (!command) {
        std::cerr << "HistoryManager: Null command passed to executeCommand" << std::endl;
        return false;
    }
    
    std::cout << "HistoryManager: Executing command: " << command->getDescription() << std::endl;
    
    // Try to execute the command
    if (!command->execute()) {
        std::cerr << "HistoryManager: Command execution failed: " << command->getDescription() << std::endl;
        return false;
    }
    
    // Clear redo stack when a new command is executed
    // (standard undo/redo behavior)
    while (!redoStack_.empty()) {
        redoStack_.pop();
    }
    
    // Add command to undo stack
    undoStack_.push(std::move(command));
    std::cout << "HistoryManager: Command added to undo stack. Stack size: " << undoStack_.size() << std::endl;
    
    // Enforce maximum history size
    enforceMaxSize();
    
    return true;
}

bool HistoryManager::undo()
{
    if (!canUndo())
        return false;
    
    // Get the command from the top of the undo stack
    auto command = std::move(undoStack_.top());
    undoStack_.pop();
    
    // Try to undo the command
    if (!command->undo()) {
        std::cerr << "HistoryManager: Undo failed for command: " << command->getDescription() << std::endl;
        // Put the command back on the undo stack since undo failed
        undoStack_.push(std::move(command));
        return false;
    }
    
    // Move command to redo stack
    redoStack_.push(std::move(command));
    
    return true;
}

bool HistoryManager::redo()
{
    if (!canRedo())
        return false;
    
    // Get the command from the top of the redo stack
    auto command = std::move(redoStack_.top());
    redoStack_.pop();
    
    // Try to re-execute the command
    if (!command->execute()) {
        std::cerr << "HistoryManager: Redo failed for command: " << command->getDescription() << std::endl;
        // Put the command back on the redo stack since redo failed
        redoStack_.push(std::move(command));
        return false;
    }
    
    // Move command back to undo stack
    undoStack_.push(std::move(command));
    
    return true;
}

bool HistoryManager::canUndo() const
{
    return !undoStack_.empty();
}

bool HistoryManager::canRedo() const
{
    return !redoStack_.empty();
}

void HistoryManager::clearHistory()
{
    while (!undoStack_.empty()) {
        undoStack_.pop();
    }
    while (!redoStack_.empty()) {
        redoStack_.pop();
    }
}

size_t HistoryManager::getUndoCount() const
{
    return undoStack_.size();
}

size_t HistoryManager::getRedoCount() const
{
    return redoStack_.size();
}

std::string HistoryManager::getNextUndoDescription() const
{
    if (canUndo())
        return undoStack_.top()->getDescription();
    return "";
}

std::string HistoryManager::getNextRedoDescription() const
{
    if (canRedo())
        return redoStack_.top()->getDescription();
    return "";
}

void HistoryManager::enforceMaxSize()
{
    // Remove old commands from the bottom of the stack if we exceed max size
    if (undoStack_.size() > maxHistorySize_) {
        // We need to rebuild the stack without the bottom elements
        std::stack<CommandPtr> tempStack;
        
        // Move commands to temp stack (this reverses the order)
        while (!undoStack_.empty()) {
            tempStack.push(std::move(undoStack_.top()));
            undoStack_.pop();
        }
        
        // Skip the oldest commands (bottom of original stack, top of temp stack)
        size_t commandsToKeep = maxHistorySize_;
        size_t commandsToSkip = tempStack.size() - commandsToKeep;
        
        for (size_t i = 0; i < commandsToSkip; ++i) {
            tempStack.pop();
        }
        
        // Move remaining commands back to undo stack (restores correct order)
        while (!tempStack.empty()) {
            undoStack_.push(std::move(tempStack.top()));
            tempStack.pop();
        }
    }
}

} // namespace EpiGimp