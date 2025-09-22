#include <gtest/gtest.h>
#include <memory>
#include <string>
#include "Core/HistoryManager.hpp"
#include "Core/ICommand.hpp"

using namespace EpiGimp;

// Simple mock command for testing HistoryManager
class TestCommand : public ICommand {
private:
    std::string description_;
    bool executed_;
    bool undone_;
    
public:
    TestCommand(const std::string& desc) : description_(desc), executed_(false), undone_(false) {}
    
    bool execute() override {
        executed_ = true;
        undone_ = false;
        return true;
    }
    
    bool undo() override {
        if (executed_) {
            undone_ = true;
            executed_ = false;
            return true;
        }
        return false;
    }
    
    std::string getDescription() const override {
        return description_;
    }
    
    // Test helpers
    bool isExecuted() const { return executed_; }
    bool isUndone() const { return undone_; }
};

class HistoryManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        manager = std::make_unique<HistoryManager>(3); // Small limit for testing
    }
    
    std::unique_ptr<HistoryManager> manager;
};

TEST_F(HistoryManagerTest, BasicFunctionality) {
    // Test initial state
    EXPECT_FALSE(manager->canUndo());
    EXPECT_FALSE(manager->canRedo());
    
    // Test executing a command
    auto command = std::make_unique<TestCommand>("Test");
    TestCommand* cmdPtr = command.get();
    
    EXPECT_TRUE(manager->executeCommand(std::move(command)));
    EXPECT_TRUE(cmdPtr->isExecuted());
    EXPECT_TRUE(manager->canUndo());
    EXPECT_FALSE(manager->canRedo());
    
    // Test undo
    EXPECT_TRUE(manager->undo());
    EXPECT_TRUE(cmdPtr->isUndone());
    EXPECT_FALSE(manager->canUndo());
    EXPECT_TRUE(manager->canRedo());
    
    // Test redo
    EXPECT_TRUE(manager->redo());
    EXPECT_TRUE(cmdPtr->isExecuted());
    EXPECT_TRUE(manager->canUndo());
    EXPECT_FALSE(manager->canRedo());
}