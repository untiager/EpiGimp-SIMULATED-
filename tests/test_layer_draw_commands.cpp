#include <gtest/gtest.h>
#include <raylib.h>
#include <memory>
#include <string>
#include <chrono>
#include <UI/Canvas.hpp>
#include <Commands/DrawCommand.hpp>
#include <Core/EventSystem.hpp>
#include <Core/HistoryManager.hpp>
#include "test_globals.hpp"

namespace EpiGimp {

class LayerDrawCommandTest : public ::testing::Test {
protected:
    std::unique_ptr<EventDispatcher> dispatcher_;
    std::unique_ptr<HistoryManager> historyManager_;
    std::unique_ptr<Canvas> canvas_;
    
    void SetUp() override {
        dispatcher_ = std::make_unique<EventDispatcher>();
        historyManager_ = std::make_unique<HistoryManager>();
        canvas_ = std::make_unique<Canvas>(
            Rectangle{0, 0, 400.0f, 300.0f}, 
            dispatcher_.get(), 
            historyManager_.get()
        );
        
        setupTestCanvas();
    }
    
    void setupTestCanvas() {
        // Create a test image and load it
        Image testImage = GenImageColor(400, 300, WHITE);
        ExportImage(testImage, "/tmp/test_draw_command.png");
        canvas_->loadImage("/tmp/test_draw_command.png");
        UnloadImage(testImage);
        
        // Create a test layer
        canvas_->addNewDrawingLayer("Test Drawing Layer");
    }
    
    void drawTestStroke() {
        // Simulate drawing a simple stroke
        canvas_->setDrawingTool(DrawingTool::Crayon);
        // Note: We can't easily simulate mouse input in tests,
        // so we'll test the command creation and execution directly
    }
    
    void drawTestStrokeWithBrush() {
        // Simulate drawing with brush tool
        canvas_->setDrawingTool(DrawingTool::Brush);
        // Note: We can't easily simulate mouse input in tests,
        // so we'll test the command creation and execution directly
    }
};

// Test DrawCommand basic functionality
TEST_F(LayerDrawCommandTest, DrawCommandCreation) {
    auto command = createDrawCommand(canvas_.get(), "Test Draw Operation");
    
    ASSERT_NE(command, nullptr);
    EXPECT_EQ(command->getDescription(), "Test Draw Operation");
}

TEST_F(LayerDrawCommandTest, DrawCommandInvalidCanvas) {
    EXPECT_THROW(createDrawCommand(nullptr, "Should Fail"), std::invalid_argument);
}

TEST_F(LayerDrawCommandTest, DrawCommandStateCapture) {
    auto command = createDrawCommand(canvas_.get(), "State Capture Test");
    
    // Capture before state
    EXPECT_NO_THROW(command->captureBeforeState());
    
    // Simulate some drawing operation here...
    // (In real usage, drawing would happen between captureBeforeState and captureAfterState)
    
    // Capture after state
    EXPECT_NO_THROW(command->captureAfterState());
}

TEST_F(LayerDrawCommandTest, DrawCommandExecution) {
    auto command = createDrawCommand(canvas_.get(), "Execution Test");
    
    command->captureBeforeState();
    
    // Simulate drawing (we can't easily do actual drawing in tests)
    // In real usage, the drawing would modify the layer texture
    
    command->captureAfterState();
    
    // Execute should succeed
    EXPECT_TRUE(command->execute());
}

TEST_F(LayerDrawCommandTest, DrawCommandUndo) {
    auto command = createDrawCommand(canvas_.get(), "Undo Test");
    
    command->captureBeforeState();
    
    // Simulate drawing operation
    // (would modify layer state here)
    
    command->captureAfterState();
    
    // Execute and then undo
    EXPECT_TRUE(command->execute());
    EXPECT_TRUE(command->undo());
}

TEST_F(LayerDrawCommandTest, DrawCommandWithoutStates) {
    auto command = createDrawCommand(canvas_.get(), "No States Test");
    
    // Try to execute without capturing states
    // Should handle gracefully
    EXPECT_NO_THROW(command->execute());
    EXPECT_NO_THROW(command->undo());
}

// Test integration with HistoryManager
TEST_F(LayerDrawCommandTest, HistoryManagerIntegration) {
    EXPECT_TRUE(historyManager_->canUndo() == false); // Initially no commands
    EXPECT_TRUE(historyManager_->canRedo() == false);
    
    // Create and execute a command
    auto command = createDrawCommand(canvas_.get(), "History Test");
    command->captureBeforeState();
    command->captureAfterState();
    
    // Add to history
    historyManager_->executeCommand(std::move(command));
    
    EXPECT_TRUE(historyManager_->canUndo());
    EXPECT_FALSE(historyManager_->canRedo());
    
    // Test undo
    historyManager_->undo();
    EXPECT_FALSE(historyManager_->canUndo());
    EXPECT_TRUE(historyManager_->canRedo());
    
    // Test redo
    historyManager_->redo();
    EXPECT_TRUE(historyManager_->canUndo());
    EXPECT_FALSE(historyManager_->canRedo());
}

TEST_F(LayerDrawCommandTest, MultipleDrawCommands) {
    const int NUM_COMMANDS = 5;
    
    // Execute multiple drawing commands
    for (int i = 0; i < NUM_COMMANDS; ++i) {
        auto command = createDrawCommand(canvas_.get(), "Command " + std::to_string(i));
        command->captureBeforeState();
        
        // Simulate drawing operation
        
        command->captureAfterState();
        historyManager_->executeCommand(std::move(command));
    }
    
    EXPECT_TRUE(historyManager_->canUndo());
    EXPECT_FALSE(historyManager_->canRedo());
    
    // Undo all commands
    for (int i = 0; i < NUM_COMMANDS; ++i) {
        EXPECT_TRUE(historyManager_->canUndo());
        historyManager_->undo();
    }
    
    EXPECT_FALSE(historyManager_->canUndo());
    EXPECT_TRUE(historyManager_->canRedo());
    
    // Redo all commands
    for (int i = 0; i < NUM_COMMANDS; ++i) {
        EXPECT_TRUE(historyManager_->canRedo());
        historyManager_->redo();
    }
    
    EXPECT_TRUE(historyManager_->canUndo());
    EXPECT_FALSE(historyManager_->canRedo());
}

// Test DrawCommand with different layer states
TEST_F(LayerDrawCommandTest, DrawCommandWithLayerChanges) {
    // Create multiple layers
    int layer1 = canvas_->addNewDrawingLayer("Layer 1");
    int layer2 = canvas_->addNewDrawingLayer("Layer 2");
    
    // Test command on layer 1
    canvas_->setSelectedLayerIndex(layer1);
    auto command1 = createDrawCommand(canvas_.get(), "Draw on Layer 1");
    command1->captureBeforeState();
    command1->captureAfterState();
    historyManager_->executeCommand(std::move(command1));
    
    // Test command on layer 2
    canvas_->setSelectedLayerIndex(layer2);
    auto command2 = createDrawCommand(canvas_.get(), "Draw on Layer 2");
    command2->captureBeforeState();
    command2->captureAfterState();
    historyManager_->executeCommand(std::move(command2));
    
    // Test undo operations
    EXPECT_TRUE(historyManager_->canUndo());
    historyManager_->undo(); // Undo layer 2 operation
    
    EXPECT_TRUE(historyManager_->canUndo());
    historyManager_->undo(); // Undo layer 1 operation
    
    EXPECT_FALSE(historyManager_->canUndo());
}

TEST_F(LayerDrawCommandTest, DrawCommandLayerVisibility) {
    // Test commands with layer visibility changes
    int layerIndex = canvas_->getSelectedLayerIndex();
    
    // Hide layer
    canvas_->setLayerVisible(layerIndex, false);
    
    // Create command with hidden layer
    auto command = createDrawCommand(canvas_.get(), "Draw on Hidden Layer");
    EXPECT_NO_THROW(command->captureBeforeState());
    EXPECT_NO_THROW(command->captureAfterState());
    
    // Show layer again
    canvas_->setLayerVisible(layerIndex, true);
    
    // Command should still work
    EXPECT_TRUE(command->execute());
    EXPECT_TRUE(command->undo());
}

TEST_F(LayerDrawCommandTest, DrawCommandAfterLayerDeletion) {
    int originalLayer = canvas_->getSelectedLayerIndex();
    
    // Create command
    auto command = createDrawCommand(canvas_.get(), "Before Layer Deletion");
    command->captureBeforeState();
    command->captureAfterState();
    
    // Delete the layer (this might invalidate the command)
    canvas_->deleteLayer(originalLayer);
    
    // Command operations should handle this gracefully
    EXPECT_NO_THROW(command->execute());
    EXPECT_NO_THROW(command->undo());
}

// Performance test for draw commands
TEST_F(LayerDrawCommandTest, DrawCommandPerformance) {
    const int NUM_COMMANDS = 20;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Create and execute many commands quickly
    for (int i = 0; i < NUM_COMMANDS; ++i) {
        auto command = createDrawCommand(canvas_.get(), "Perf Test " + std::to_string(i));
        command->captureBeforeState();
        command->captureAfterState();
        historyManager_->executeCommand(std::move(command));
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete reasonably quickly
    EXPECT_LT(duration.count(), 1000); // Less than 1 second
    
    // Test undo performance
    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_COMMANDS; ++i) {
        historyManager_->undo();
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 500); // Undo should be faster
}

// Memory management tests
TEST_F(LayerDrawCommandTest, DrawCommandMemoryManagement) {
    // Test that commands properly clean up their resources
    {
        auto command = createDrawCommand(canvas_.get(), "Memory Test");
        command->captureBeforeState();
        command->captureAfterState();
        // Command should clean up automatically when it goes out of scope
    }
    
    // Test multiple commands with automatic cleanup
    for (int i = 0; i < 10; ++i) {
        auto command = createDrawCommand(canvas_.get(), "Memory Test " + std::to_string(i));
        command->captureBeforeState();
        command->captureAfterState();
        historyManager_->executeCommand(std::move(command));
    }
    
    // Clear history - should clean up all commands
    historyManager_->clearHistory();
    EXPECT_FALSE(historyManager_->canUndo());
    EXPECT_FALSE(historyManager_->canRedo());
}

// Edge cases
TEST_F(LayerDrawCommandTest, DrawCommandEdgeCases) {
    // Test command with no selected layer
    canvas_->setSelectedLayerIndex(-1);
    auto command = createDrawCommand(canvas_.get(), "No Layer Selected");
    
    // Should handle gracefully
    EXPECT_NO_THROW(command->captureBeforeState());
    EXPECT_NO_THROW(command->captureAfterState());
    EXPECT_NO_THROW(command->execute());
    EXPECT_NO_THROW(command->undo());
    
    // Test command with empty description
    auto emptyCommand = createDrawCommand(canvas_.get(), "");
    EXPECT_NE(emptyCommand, nullptr);
    
    // Test double capture of states
    canvas_->setSelectedLayerIndex(0); // Select valid layer
    auto doubleCommand = createDrawCommand(canvas_.get(), "Double Capture");
    doubleCommand->captureBeforeState();
    doubleCommand->captureBeforeState(); // Should handle gracefully
    doubleCommand->captureAfterState();
    doubleCommand->captureAfterState(); // Should handle gracefully
}

// Brush Tool Specific Tests
TEST_F(LayerDrawCommandTest, BrushToolDrawCommand) {
    setupTestCanvas();
    
    // Set brush tool
    canvas_->setDrawingTool(DrawingTool::Brush);
    
    // Create draw command with brush tool
    auto command = createDrawCommand(canvas_.get(), "Brush Draw Command");
    
    ASSERT_NE(command, nullptr);
    EXPECT_EQ(command->getDescription(), "Brush Draw Command");
    
    // Test state capture with brush tool
    canvas_->setSelectedLayerIndex(0);
    EXPECT_NO_THROW(command->captureBeforeState());
    EXPECT_NO_THROW(command->captureAfterState());
    
    // Test execution
    EXPECT_TRUE(command->execute());
    EXPECT_TRUE(command->undo());
}

TEST_F(LayerDrawCommandTest, BrushVsCrayonToolCommands) {
    setupTestCanvas();
    canvas_->setSelectedLayerIndex(0);
    
    // Test brush tool command
    canvas_->setDrawingTool(DrawingTool::Brush);
    auto brushCommand = createDrawCommand(canvas_.get(), "Brush Command");
    EXPECT_NO_THROW(brushCommand->captureBeforeState());
    EXPECT_NO_THROW(brushCommand->captureAfterState());
    
    // Test crayon tool command
    canvas_->setDrawingTool(DrawingTool::Crayon);
    auto crayonCommand = createDrawCommand(canvas_.get(), "Crayon Command");
    EXPECT_NO_THROW(crayonCommand->captureBeforeState());
    EXPECT_NO_THROW(crayonCommand->captureAfterState());
    
    // Both should execute successfully
    EXPECT_TRUE(brushCommand->execute());
    EXPECT_TRUE(crayonCommand->execute());
    
    // Both should undo successfully
    EXPECT_TRUE(crayonCommand->undo());
    EXPECT_TRUE(brushCommand->undo());
}

TEST_F(LayerDrawCommandTest, ToolSwitchingDuringDraw) {
    setupTestCanvas();
    canvas_->setSelectedLayerIndex(0);
    
    // Start with brush tool
    canvas_->setDrawingTool(DrawingTool::Brush);
    auto command = createDrawCommand(canvas_.get(), "Tool Switch Test");
    command->captureBeforeState();
    
    // Switch tool during "drawing" (before capturing after state)
    canvas_->setDrawingTool(DrawingTool::Crayon);
    canvas_->setDrawingTool(DrawingTool::Brush);
    
    // Should still work fine
    EXPECT_NO_THROW(command->captureAfterState());
    EXPECT_TRUE(command->execute());
    EXPECT_TRUE(command->undo());
}

} // namespace EpiGimp