#include <gtest/gtest.h>
#include <raylib.h>
#include <memory>
#include <string>
#include <chrono>
#include <UI/Canvas.hpp>
#include <Core/EventSystem.hpp>
#include <Core/HistoryManager.hpp>
#include "test_globals.hpp"

namespace EpiGimp {

class CanvasLayerTest : public ::testing::Test {
protected:
    std::unique_ptr<EventDispatcher> dispatcher_;
    std::unique_ptr<HistoryManager> historyManager_;
    std::unique_ptr<Canvas> canvas_;
    
    void SetUp() override {
        dispatcher_ = std::make_unique<EventDispatcher>();
        historyManager_ = std::make_unique<HistoryManager>();
        canvas_ = std::make_unique<Canvas>(
            Rectangle{0, 0, 800.0f, 600.0f}, 
            dispatcher_.get(), 
            historyManager_.get(),
            false  // Disable auto-create blank canvas for tests
        );
    }
    
    void loadTestImage() {
        // Create a simple test image
        Image testImage = GenImageColor(800, 600, BLUE);
        Texture2D testTexture = LoadTextureFromImage(testImage);
        
        // Save it temporarily and load it through Canvas
        ExportImage(testImage, "/tmp/test_canvas_image.png");
        canvas_->loadImage("/tmp/test_canvas_image.png");
        
        UnloadImage(testImage);
        UnloadTexture(testTexture);
    }
};

// Test basic Canvas functionality
TEST_F(CanvasLayerTest, CanvasInitialization) {
    EXPECT_FALSE(canvas_->hasImage());
    EXPECT_EQ(canvas_->getLayerCount(), 0);
    EXPECT_EQ(canvas_->getSelectedLayerIndex(), -1);
    EXPECT_FLOAT_EQ(canvas_->getZoom(), 1.0f);
    
    Vector2 pan = canvas_->getPan();
    EXPECT_FLOAT_EQ(pan.x, 0.0f);
    EXPECT_FLOAT_EQ(pan.y, 0.0f);
}

TEST_F(CanvasLayerTest, ImageLoading) {
    EXPECT_FALSE(canvas_->hasImage());
    
    loadTestImage();
    
    EXPECT_TRUE(canvas_->hasImage());
}

TEST_F(CanvasLayerTest, LayerCreation) {
    loadTestImage(); // Need background image for layers
    
    int initialCount = canvas_->getLayerCount();
    
    int newLayerIndex = canvas_->addNewDrawingLayer("Test Layer");
    
    EXPECT_NE(newLayerIndex, -1);
    EXPECT_EQ(canvas_->getLayerCount(), initialCount + 1);
    EXPECT_EQ(canvas_->getSelectedLayerIndex(), newLayerIndex);
    
    const std::string& layerName = canvas_->getLayerName(newLayerIndex);
    EXPECT_EQ(layerName, "Test Layer");
}

TEST_F(CanvasLayerTest, LayerCreationWithoutBackground) {
    // Should fail without background image
    int layerIndex = canvas_->addNewDrawingLayer("Should Fail");
    EXPECT_EQ(layerIndex, -1);
}

TEST_F(CanvasLayerTest, AutoLayerNaming) {
    loadTestImage();
    
    // Create layers without names
    int layer1 = canvas_->addNewDrawingLayer();
    int layer2 = canvas_->addNewDrawingLayer();
    int layer3 = canvas_->addNewDrawingLayer();
    
    EXPECT_NE(layer1, -1);
    EXPECT_NE(layer2, -1);
    EXPECT_NE(layer3, -1);
    
    // Check unique names were generated
    std::string name1 = canvas_->getLayerName(layer1);
    std::string name2 = canvas_->getLayerName(layer2);
    std::string name3 = canvas_->getLayerName(layer3);
    
    EXPECT_NE(name1, name2);
    EXPECT_NE(name2, name3);
    EXPECT_NE(name1, name3);
    
    // Names should follow pattern "Layer X"
    EXPECT_TRUE(name1.find("Layer") != std::string::npos);
    EXPECT_TRUE(name2.find("Layer") != std::string::npos);
    EXPECT_TRUE(name3.find("Layer") != std::string::npos);
}

TEST_F(CanvasLayerTest, LayerSelection) {
    loadTestImage();
    
    int layer1 = canvas_->addNewDrawingLayer("Layer 1");
    int layer2 = canvas_->addNewDrawingLayer("Layer 2");
    
    // Layer 2 should be selected by default (last created)
    EXPECT_EQ(canvas_->getSelectedLayerIndex(), layer2);
    
    // Test selecting layer 1
    canvas_->setSelectedLayerIndex(layer1);
    EXPECT_EQ(canvas_->getSelectedLayerIndex(), layer1);
    
    // Test invalid selection
    canvas_->setSelectedLayerIndex(100);
    EXPECT_EQ(canvas_->getSelectedLayerIndex(), layer1); // Should remain unchanged
    
    // Test selecting -1 (no layer)
    canvas_->setSelectedLayerIndex(-1);
    EXPECT_EQ(canvas_->getSelectedLayerIndex(), -1);
}

TEST_F(CanvasLayerTest, LayerDeletion) {
    loadTestImage();
    
    int layer1 = canvas_->addNewDrawingLayer("Layer 1");
    int layer2 = canvas_->addNewDrawingLayer("Layer 2");
    int initialCount = canvas_->getLayerCount();
    
    // Delete layer 1
    canvas_->deleteLayer(layer1);
    
    EXPECT_EQ(canvas_->getLayerCount(), initialCount - 1);
    
    // Try to delete invalid layer
    int oldCount = canvas_->getLayerCount();
    canvas_->deleteLayer(100);
    EXPECT_EQ(canvas_->getLayerCount(), oldCount); // Should remain unchanged
}

TEST_F(CanvasLayerTest, LayerVisibility) {
    loadTestImage();
    
    int layerIndex = canvas_->addNewDrawingLayer("Visibility Test");
    
    // Layer should be visible by default
    EXPECT_TRUE(canvas_->isLayerVisible(layerIndex));
    
    // Hide layer
    canvas_->setLayerVisible(layerIndex, false);
    EXPECT_FALSE(canvas_->isLayerVisible(layerIndex));
    
    // Show layer again
    canvas_->setLayerVisible(layerIndex, true);
    EXPECT_TRUE(canvas_->isLayerVisible(layerIndex));
    
    // Test invalid layer
    EXPECT_FALSE(canvas_->isLayerVisible(100));
}

TEST_F(CanvasLayerTest, LayerClearing) {
    loadTestImage();
    
    int layerIndex = canvas_->addNewDrawingLayer("Clear Test");
    
    // Clear layer should not crash
    EXPECT_NO_THROW(canvas_->clearLayer(layerIndex));
    
    // Clear invalid layer should not crash
    EXPECT_NO_THROW(canvas_->clearLayer(100));
    
    // Clear selected layer
    canvas_->setSelectedLayerIndex(layerIndex);
    EXPECT_NO_THROW(canvas_->clearDrawingLayer());
}

TEST_F(CanvasLayerTest, LayerAccess) {
    loadTestImage();
    
    int layerIndex = canvas_->addNewDrawingLayer("Access Test");
    
    // Test const access
    const DrawingLayer* constLayer = canvas_->getLayer(layerIndex);
    ASSERT_NE(constLayer, nullptr);
    EXPECT_EQ(constLayer->name, "Access Test");
    EXPECT_TRUE(constLayer->visible);
    
    // Test non-const access
    DrawingLayer* layer = canvas_->getLayer(layerIndex);
    ASSERT_NE(layer, nullptr);
    EXPECT_EQ(layer->name, "Access Test");
    
    // Test invalid access
    EXPECT_EQ(canvas_->getLayer(100), nullptr);
    EXPECT_EQ(canvas_->getLayer(-5), nullptr);
}

TEST_F(CanvasLayerTest, DrawingTexture) {
    loadTestImage();
    
    int layerIndex = canvas_->addNewDrawingLayer("Drawing Test");
    
    // Should have drawing texture after creation
    EXPECT_TRUE(canvas_->hasDrawingTexture());
    
    // Test copying drawing image
    Image copiedImage = canvas_->copyDrawingImage();
    EXPECT_GT(copiedImage.width, 0);
    EXPECT_GT(copiedImage.height, 0);
    UnloadImage(copiedImage);
    
    // Test without selected layer
    canvas_->setSelectedLayerIndex(-1);
    EXPECT_FALSE(canvas_->hasDrawingTexture());
}

TEST_F(CanvasLayerTest, ResetToBackground) {
    loadTestImage();
    
    // Create some layers
    canvas_->addNewDrawingLayer("Layer 1");
    canvas_->addNewDrawingLayer("Layer 2");
    
    int layerCount = canvas_->getLayerCount();
    EXPECT_GT(layerCount, 0);
    
    // Reset to background
    canvas_->resetToBackground();
    
    EXPECT_EQ(canvas_->getLayerCount(), 0);
    EXPECT_EQ(canvas_->getSelectedLayerIndex(), -1);
    EXPECT_TRUE(canvas_->isBackgroundVisible());
}

TEST_F(CanvasLayerTest, BackgroundVisibility) {
    // Background should be visible by default
    EXPECT_TRUE(canvas_->isBackgroundVisible());
    
    // Hide background
    canvas_->setBackgroundVisible(false);
    EXPECT_FALSE(canvas_->isBackgroundVisible());
    
    // Show background again
    canvas_->setBackgroundVisible(true);
    EXPECT_TRUE(canvas_->isBackgroundVisible());
}

TEST_F(CanvasLayerTest, ZoomAndPan) {
    // Test zoom
    canvas_->setZoom(2.0f);
    EXPECT_FLOAT_EQ(canvas_->getZoom(), 2.0f);
    
    canvas_->setZoom(0.5f);
    EXPECT_FLOAT_EQ(canvas_->getZoom(), 0.5f);
    
    // Test zoom limits (if implemented)
    canvas_->setZoom(100.0f);
    float zoom = canvas_->getZoom();
    EXPECT_LE(zoom, 10.0f); // Should be clamped to reasonable value
    
    canvas_->setZoom(0.01f);
    zoom = canvas_->getZoom();
    EXPECT_GE(zoom, 0.05f); // Should be clamped to reasonable value
    
    // Test pan
    Vector2 newPan = {100.0f, 50.0f};
    canvas_->setPan(newPan);
    Vector2 currentPan = canvas_->getPan();
    EXPECT_FLOAT_EQ(currentPan.x, newPan.x);
    EXPECT_FLOAT_EQ(currentPan.y, newPan.y);
}

TEST_F(CanvasLayerTest, DrawingTool) {
    // Test setting drawing tool
    canvas_->setDrawingTool(DrawingTool::Crayon);
    // Note: Canvas doesn't expose current tool, so we can't directly test
    // But the call should not crash
    EXPECT_NO_THROW(canvas_->setDrawingTool(DrawingTool::None));
}

// Performance test
TEST_F(CanvasLayerTest, LayerPerformance) {
    loadTestImage();
    
    const int NUM_LAYERS = 50;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Create many layers
    for (int i = 0; i < NUM_LAYERS; ++i) {
        int index = canvas_->addNewDrawingLayer("Performance Layer " + std::to_string(i));
        EXPECT_NE(index, -1);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 2000); // Should complete in less than 2 seconds
    EXPECT_EQ(canvas_->getLayerCount(), NUM_LAYERS);
    
    // Test accessing layers quickly
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_LAYERS; ++i) {
        const DrawingLayer* layer = canvas_->getLayer(i);
        EXPECT_NE(layer, nullptr);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 10); // Should be very fast
}

// Edge cases and error conditions
TEST_F(CanvasLayerTest, EdgeCases) {
    // Operations without image
    EXPECT_EQ(canvas_->addNewDrawingLayer(), -1);
    EXPECT_FALSE(canvas_->hasDrawingTexture());
    
    // Operations on empty layer list
    EXPECT_EQ(canvas_->getLayerName(-1), "");
    EXPECT_EQ(canvas_->getLayerName(0), "");
    EXPECT_FALSE(canvas_->isLayerVisible(0));
    
    // After loading image but no layers
    loadTestImage();
    EXPECT_EQ(canvas_->getLayerCount(), 0);
    EXPECT_FALSE(canvas_->hasDrawingTexture());
    
    // Test empty layer name
    int index = canvas_->addNewDrawingLayer("");
    EXPECT_NE(index, -1);
    std::string name = canvas_->getLayerName(index);
    EXPECT_FALSE(name.empty()); // Should get auto-generated name
}

} // namespace EpiGimp