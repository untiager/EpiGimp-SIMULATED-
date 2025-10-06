//Test file for brush tool functionality
#include <gtest/gtest.h>
#include <memory>
#include "../include/UI/Canvas.hpp"
#include "../include/UI/Toolbar.hpp"
#include "../include/Core/EventSystem.hpp"
#include "../include/Core/HistoryManager.hpp"
#include "../include/Core/Interfaces.hpp"
#include "../include/Commands/DrawCommand.hpp"
#include "test_globals.hpp"

namespace EpiGimp {

class BrushToolTest : public ::testing::Test {
protected:
    void SetUp() override {
        eventDispatcher_ = std::make_unique<EventDispatcher>();
        historyManager_ = std::make_unique<HistoryManager>();
        
        Rectangle canvasBounds = {0, 0, 800, 600};
        canvas_ = std::make_unique<Canvas>(canvasBounds, eventDispatcher_.get(), historyManager_.get());
        
        Rectangle toolbarBounds = {0, 0, 800, 60};
        toolbar_ = std::make_unique<Toolbar>(toolbarBounds, eventDispatcher_.get());
        
        // Create test image
        createTestImage();
    }
    
    void TearDown() override {
        canvas_.reset();
        toolbar_.reset();
        historyManager_.reset();
        eventDispatcher_.reset();
    }
    
    void createTestImage() {
        Image testImage = GenImageColor(400, 300, WHITE);
        ExportImage(testImage, "/tmp/brush_test_image.png");
        canvas_->loadImage("/tmp/brush_test_image.png");
        UnloadImage(testImage);
        
        // Create a drawing layer
        int layerIndex = canvas_->addNewDrawingLayer("Brush Test Layer");
        canvas_->setSelectedLayerIndex(layerIndex);
    }
    
    std::unique_ptr<Canvas> canvas_;
    std::unique_ptr<Toolbar> toolbar_;
    std::unique_ptr<EventDispatcher> eventDispatcher_;
    std::unique_ptr<HistoryManager> historyManager_;
};

// Test basic brush tool functionality
TEST_F(BrushToolTest, BasicBrushToolSetup) {
    // Test setting brush tool
    EXPECT_NO_THROW(canvas_->setDrawingTool(DrawingTool::Brush));
    
    // Verify canvas is ready for drawing
    EXPECT_TRUE(canvas_->hasImage());
    EXPECT_TRUE(canvas_->hasDrawingTexture());
}

// Test brush tool enum validation
TEST_F(BrushToolTest, BrushToolEnumValidation) {
    // Test all enum values
    EXPECT_NO_THROW(canvas_->setDrawingTool(DrawingTool::None));
    EXPECT_NO_THROW(canvas_->setDrawingTool(DrawingTool::Crayon));
    EXPECT_NO_THROW(canvas_->setDrawingTool(DrawingTool::Brush));
    
    // Test that brush is a valid enum value
    DrawingTool brushTool = DrawingTool::Brush;
    EXPECT_NE(brushTool, DrawingTool::None);
    EXPECT_NE(brushTool, DrawingTool::Crayon);
}

// Test tool switching scenarios
TEST_F(BrushToolTest, ToolSwitching) {
    // Switch between tools multiple times
    canvas_->setDrawingTool(DrawingTool::None);
    canvas_->setDrawingTool(DrawingTool::Brush);
    canvas_->setDrawingTool(DrawingTool::Crayon);
    canvas_->setDrawingTool(DrawingTool::Brush);
    
    // Should not crash or cause issues
    EXPECT_TRUE(canvas_->hasImage());
    EXPECT_TRUE(canvas_->hasDrawingTexture());
}

// Test brush tool with toolbar integration
TEST_F(BrushToolTest, ToolbarIntegration) {
    // Test toolbar button addition
    bool brushButtonClicked = false;
    toolbar_->addButton("Brush", [&brushButtonClicked, this]() {
        brushButtonClicked = true;
        eventDispatcher_->emit<ToolSelectedEvent>(DrawingTool::Brush);
    });
    
    // Test event emission
    bool eventReceived = false;
    eventDispatcher_->subscribe<ToolSelectedEvent>([&eventReceived, this](const ToolSelectedEvent& event) {
        eventReceived = true;
        EXPECT_EQ(event.toolType, DrawingTool::Brush);
        canvas_->setDrawingTool(event.toolType);
    });
    
    // Simulate button click by emitting event
    eventDispatcher_->emit<ToolSelectedEvent>(DrawingTool::Brush);
    
    EXPECT_TRUE(eventReceived);
}

// Test brush tool with layer system
TEST_F(BrushToolTest, LayerSystemIntegration) {
    // Set brush tool
    canvas_->setDrawingTool(DrawingTool::Brush);
    
    // Test with multiple layers
    int layer1 = canvas_->addNewDrawingLayer("Brush Layer 1");
    int layer2 = canvas_->addNewDrawingLayer("Brush Layer 2");
    
    // Switch between layers with brush tool
    canvas_->setSelectedLayerIndex(layer1);
    EXPECT_EQ(canvas_->getSelectedLayerIndex(), layer1);
    
    canvas_->setSelectedLayerIndex(layer2);
    EXPECT_EQ(canvas_->getSelectedLayerIndex(), layer2);
    
    // Tool should work with any selected layer
    EXPECT_TRUE(canvas_->hasDrawingTexture());
}

// Test brush tool with history system
TEST_F(BrushToolTest, HistorySystemIntegration) {
    canvas_->setDrawingTool(DrawingTool::Brush);
    
    // Create a draw command (simulating a brush stroke)
    auto command = createDrawCommand(canvas_.get(), "Brush Stroke");
    ASSERT_NE(command, nullptr);
    
    // Test state capture
    EXPECT_NO_THROW(command->captureBeforeState());
    EXPECT_NO_THROW(command->captureAfterState());
    
    // Test execution through history manager
    bool commandExecuted = historyManager_->executeCommand(std::move(command));
    EXPECT_TRUE(commandExecuted);
    
    // Test undo
    bool undoSuccessful = historyManager_->undo();
    EXPECT_TRUE(undoSuccessful);
}

// Test brush tool stability under stress
TEST_F(BrushToolTest, StressTest) {
    // Rapidly switch to brush tool many times
    for (int i = 0; i < 100; ++i) {
        canvas_->setDrawingTool(DrawingTool::Brush);
        canvas_->setDrawingTool(DrawingTool::Crayon);
    }
    
    // End with brush tool
    canvas_->setDrawingTool(DrawingTool::Brush);
    
    // Should still work fine
    EXPECT_TRUE(canvas_->hasImage());
    EXPECT_TRUE(canvas_->hasDrawingTexture());
}

// Test brush tool with different canvas states
TEST_F(BrushToolTest, CanvasStateCompatibility) {
    // Test with no image loaded
    Canvas emptyCanvas({0, 0, 400, 300}, eventDispatcher_.get(), historyManager_.get());
    EXPECT_NO_THROW(emptyCanvas.setDrawingTool(DrawingTool::Brush));
    
    // Test with image but no layers
    emptyCanvas.loadImage("/tmp/brush_test_image.png");
    EXPECT_NO_THROW(emptyCanvas.setDrawingTool(DrawingTool::Brush));
    
    // Test with multiple layers
    emptyCanvas.addNewDrawingLayer("Layer 1");
    emptyCanvas.addNewDrawingLayer("Layer 2");
    emptyCanvas.setSelectedLayerIndex(1);
    EXPECT_NO_THROW(emptyCanvas.setDrawingTool(DrawingTool::Brush));
}

// Test brush tool persistence across operations
TEST_F(BrushToolTest, ToolPersistence) {
    // Set brush tool
    canvas_->setDrawingTool(DrawingTool::Brush);
    
    // Perform various operations
    canvas_->addNewDrawingLayer("New Layer");
    canvas_->setZoom(2.0f);
    canvas_->setPan({10.0f, 20.0f});
    
    // Tool should still be usable (we can't directly check the internal state,
    // but we can verify that setting it again doesn't cause issues)
    EXPECT_NO_THROW(canvas_->setDrawingTool(DrawingTool::Brush));
}

} // namespace EpiGimp