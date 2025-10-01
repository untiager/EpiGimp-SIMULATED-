#include <gtest/gtest.h>
#include <raylib.h>
#include <memory>
#include <string>
#include <chrono>
#include <Core/LayerManager.hpp>
#include <Core/Layer.hpp>
#include <Core/EventSystem.hpp>
#include "test_globals.hpp"

namespace EpiGimp {

class LayerSystemTest : public ::testing::Test {
protected:
    std::unique_ptr<EventDispatcher> dispatcher_;
    std::unique_ptr<LayerManager> layerManager_;
    
    void SetUp() override {
        dispatcher_ = std::make_unique<EventDispatcher>();
        layerManager_ = std::make_unique<LayerManager>(800, 600, dispatcher_.get());
    }
};

// Test Layer class basic functionality
TEST_F(LayerSystemTest, LayerCreation) {
    Layer layer("Test Layer", 800, 600);
    
    EXPECT_EQ(layer.getName(), "Test Layer");
    EXPECT_TRUE(layer.isVisible());
    EXPECT_FLOAT_EQ(layer.getOpacity(), 1.0f);
    EXPECT_EQ(layer.getBlendMode(), BlendMode::Normal);
    EXPECT_EQ(layer.getWidth(), 800);
    EXPECT_EQ(layer.getHeight(), 600);
    EXPECT_TRUE(layer.hasTexture());
}

TEST_F(LayerSystemTest, LayerInvalidDimensions) {
    EXPECT_THROW(Layer("Invalid Layer", 0, 100), std::invalid_argument);
    EXPECT_THROW(Layer("Invalid Layer", 100, 0), std::invalid_argument);
    EXPECT_THROW(Layer("Invalid Layer", -10, 100), std::invalid_argument);
}

TEST_F(LayerSystemTest, LayerProperties) {
    Layer layer("Test Layer", 800, 600);
    
    // Test visibility
    layer.setVisible(false);
    EXPECT_FALSE(layer.isVisible());
    layer.setVisible(true);
    EXPECT_TRUE(layer.isVisible());
    
    // Test opacity
    layer.setOpacity(0.5f);
    EXPECT_FLOAT_EQ(layer.getOpacity(), 0.5f);
    
    // Test opacity clamping
    layer.setOpacity(-0.5f);
    EXPECT_FLOAT_EQ(layer.getOpacity(), 0.0f);
    layer.setOpacity(1.5f);
    EXPECT_FLOAT_EQ(layer.getOpacity(), 1.0f);
    
    // Test blend mode
    layer.setBlendMode(BlendMode::Multiply);
    EXPECT_EQ(layer.getBlendMode(), BlendMode::Multiply);
    
    // Test name change
    layer.setName("New Name");
    EXPECT_EQ(layer.getName(), "New Name");
}

TEST_F(LayerSystemTest, LayerDrawingOperations) {
    Layer layer("Drawing Test", 800, 600);
    
    // Test drawing operations don't crash
    EXPECT_NO_THROW({
        layer.beginDrawing();
        DrawRectangle(10, 10, 50, 50, RED);
        layer.endDrawing();
    });
    
    // Test clear operation
    EXPECT_NO_THROW(layer.clear(BLUE));
    
    // Test image copy
    Image copiedImage = layer.copyImage();
    EXPECT_GT(copiedImage.width, 0);
    EXPECT_GT(copiedImage.height, 0);
    UnloadImage(copiedImage);
}

// Test LayerManager basic functionality
TEST_F(LayerSystemTest, LayerManagerCreation) {
    EXPECT_EQ(layerManager_->getLayerCount(), 1); // Should have default layer
    EXPECT_EQ(layerManager_->getActiveLayerIndex(), 0);
    EXPECT_NE(layerManager_->getActiveLayer(), nullptr);
}

TEST_F(LayerSystemTest, LayerManagerInvalidDimensions) {
    EXPECT_THROW(LayerManager(-10, 100, dispatcher_.get()), std::invalid_argument);
    EXPECT_THROW(LayerManager(100, 0, dispatcher_.get()), std::invalid_argument);
}

TEST_F(LayerSystemTest, LayerManagerCreateLayer) {
    size_t initialCount = layerManager_->getLayerCount();
    
    size_t newLayerIndex = layerManager_->createLayer("New Layer");
    
    EXPECT_EQ(layerManager_->getLayerCount(), initialCount + 1);
    EXPECT_EQ(newLayerIndex, initialCount);
    
    const Layer* newLayer = layerManager_->getLayer(newLayerIndex);
    ASSERT_NE(newLayer, nullptr);
    EXPECT_EQ(newLayer->getName(), "New Layer");
}

TEST_F(LayerSystemTest, LayerManagerDeleteLayer) {
    // Create multiple layers
    layerManager_->createLayer("Layer 1");
    layerManager_->createLayer("Layer 2");
    size_t initialCount = layerManager_->getLayerCount();
    
    // Delete middle layer
    bool success = layerManager_->deleteLayer(1);
    EXPECT_TRUE(success);
    EXPECT_EQ(layerManager_->getLayerCount(), initialCount - 1);
    
    // Try to delete invalid index
    success = layerManager_->deleteLayer(100);
    EXPECT_FALSE(success);
    
    // Cannot delete the last layer
    while (layerManager_->getLayerCount() > 1) {
        layerManager_->deleteLayer(0);
    }
    success = layerManager_->deleteLayer(0);
    EXPECT_FALSE(success);
    EXPECT_EQ(layerManager_->getLayerCount(), 1);
}

// Note: LayerManager::moveLayer is not implemented yet, so this test is commented out
// TEST_F(LayerSystemTest, LayerManagerMoveLayer) {
//     // Create test layers
//     layerManager_->createLayer("Layer 1");
//     layerManager_->createLayer("Layer 2");
//     layerManager_->createLayer("Layer 3");
//     
//     size_t layerCount = layerManager_->getLayerCount();
//     
//     // Move layer from index 1 to index 3
//     bool success = layerManager_->moveLayer(1, 3);
//     EXPECT_TRUE(success);
//     EXPECT_EQ(layerManager_->getLayerCount(), layerCount); // Count should remain the same
//     
//     // Try invalid moves
//     success = layerManager_->moveLayer(100, 0);
//     EXPECT_FALSE(success);
//     success = layerManager_->moveLayer(0, 100);
//     EXPECT_FALSE(success);
//     success = layerManager_->moveLayer(1, 1); // Move to same position
//     EXPECT_TRUE(success); // Should succeed but do nothing
// }

TEST_F(LayerSystemTest, LayerManagerDuplicateLayer) {
    size_t originalIndex = layerManager_->createLayer("Original Layer");
    size_t initialCount = layerManager_->getLayerCount();
    
    bool success = layerManager_->duplicateLayer(originalIndex);
    EXPECT_TRUE(success);
    EXPECT_EQ(layerManager_->getLayerCount(), initialCount + 1);
    
    // Try to duplicate invalid index
    success = layerManager_->duplicateLayer(100);
    EXPECT_FALSE(success);
}

TEST_F(LayerSystemTest, LayerManagerActiveLayerManagement) {
    layerManager_->createLayer("Layer 1");
    layerManager_->createLayer("Layer 2");
    
    // Test setting active layer
    bool success = layerManager_->setActiveLayer(1);
    EXPECT_TRUE(success);
    EXPECT_EQ(layerManager_->getActiveLayerIndex(), 1);
    
    // Test invalid active layer
    success = layerManager_->setActiveLayer(100);
    EXPECT_FALSE(success);
    EXPECT_EQ(layerManager_->getActiveLayerIndex(), 1); // Should remain unchanged
}

TEST_F(LayerSystemTest, LayerManagerLayerProperties) {
    size_t layerIndex = layerManager_->createLayer("Property Test");
    
    // Test visibility
    bool success = layerManager_->setLayerVisibility(layerIndex, false);
    EXPECT_TRUE(success);
    const Layer* layer = layerManager_->getLayer(layerIndex);
    ASSERT_NE(layer, nullptr);
    EXPECT_FALSE(layer->isVisible());
    
    // Test opacity
    success = layerManager_->setLayerOpacity(layerIndex, 0.7f);
    EXPECT_TRUE(success);
    EXPECT_FLOAT_EQ(layer->getOpacity(), 0.7f);
    
    // Test blend mode
    success = layerManager_->setLayerBlendMode(layerIndex, BlendMode::Multiply);
    EXPECT_TRUE(success);
    EXPECT_EQ(layer->getBlendMode(), BlendMode::Multiply);
    
    // Test name change
    success = layerManager_->setLayerName(layerIndex, "New Property Name");
    EXPECT_TRUE(success);
    EXPECT_EQ(layer->getName(), "New Property Name");
    
    // Test invalid operations
    success = layerManager_->setLayerVisibility(100, true);
    EXPECT_FALSE(success);
    success = layerManager_->setLayerOpacity(100, 0.5f);
    EXPECT_FALSE(success);
}

TEST_F(LayerSystemTest, LayerManagerUtilityFunctions) {
    layerManager_->createLayer("Utility Layer 1");
    layerManager_->createLayer("Utility Layer 2");
    
    size_t layerCountBefore = layerManager_->getLayerCount();
    
    // Test layer names
    std::vector<std::string> names = layerManager_->getLayerNames();
    EXPECT_GE(names.size(), 3); // At least default + 2 created
    
    // Test find by name
    int foundIndex = layerManager_->findLayerByName("Utility Layer 1");
    EXPECT_NE(foundIndex, -1);
    
    foundIndex = layerManager_->findLayerByName("Nonexistent Layer");
    EXPECT_EQ(foundIndex, -1);
    
    // Test clear - this clears layer content, not removes layers
    layerManager_->clear();
    EXPECT_EQ(layerManager_->getLayerCount(), layerCountBefore); // Layer count should remain the same
    
    // Test that layers still exist but are cleared
    const Layer* layer1 = layerManager_->getLayer(1);
    ASSERT_NE(layer1, nullptr);
    EXPECT_EQ(layer1->getName(), "Utility Layer 1"); // Name should still be there
}

TEST_F(LayerSystemTest, LayerManagerResizeOperations) {
    layerManager_->createLayer("Resize Test");
    
    // Test resize all layers
    EXPECT_NO_THROW(layerManager_->resizeAllLayers(400, 300));
    
    // Verify layer dimensions (if accessible through Layer interface)
    const Layer* layer = layerManager_->getLayer(0);
    ASSERT_NE(layer, nullptr);
    EXPECT_EQ(layer->getWidth(), 400);
    EXPECT_EQ(layer->getHeight(), 300);
}

// Test layer event system
class LayerEventTest : public LayerSystemTest {
protected:
    std::vector<std::unique_ptr<Event>> capturedEvents_;
    
    void SetUp() override {
        LayerSystemTest::SetUp();
        
        // Subscribe to layer events
        dispatcher_->subscribe<LayerCreatedEvent>([this](const LayerCreatedEvent& event) {
            capturedEvents_.push_back(std::make_unique<LayerCreatedEvent>(event));
        });
        
        dispatcher_->subscribe<LayerDeletedEvent>([this](const LayerDeletedEvent& event) {
            capturedEvents_.push_back(std::make_unique<LayerDeletedEvent>(event));
        });
        
        dispatcher_->subscribe<ActiveLayerChangedEvent>([this](const ActiveLayerChangedEvent& event) {
            capturedEvents_.push_back(std::make_unique<ActiveLayerChangedEvent>(event));
        });
    }
};

TEST_F(LayerEventTest, LayerCreationEvents) {
    capturedEvents_.clear();
    
    layerManager_->createLayer("Event Test Layer");
    
    EXPECT_EQ(capturedEvents_.size(), 1);
    if (!capturedEvents_.empty()) {
        auto* createdEvent = dynamic_cast<LayerCreatedEvent*>(capturedEvents_[0].get());
        ASSERT_NE(createdEvent, nullptr);
        EXPECT_GE(createdEvent->layerIndex, 0);
    }
}

TEST_F(LayerEventTest, LayerDeletionEvents) {
    size_t layerIndex = layerManager_->createLayer("Delete Test Layer");
    capturedEvents_.clear();
    
    layerManager_->deleteLayer(layerIndex);
    
    EXPECT_GE(capturedEvents_.size(), 1); // Should have deletion event, possibly active layer change
    bool foundDeleteEvent = false;
    for (const auto& event : capturedEvents_) {
        if (dynamic_cast<LayerDeletedEvent*>(event.get())) {
            foundDeleteEvent = true;
            break;
        }
    }
    EXPECT_TRUE(foundDeleteEvent);
}

TEST_F(LayerEventTest, ActiveLayerChangeEvents) {
    layerManager_->createLayer("Active Test Layer");
    capturedEvents_.clear();
    
    layerManager_->setActiveLayer(1);
    
    EXPECT_GE(capturedEvents_.size(), 1);
    bool foundActiveChangeEvent = false;
    for (const auto& event : capturedEvents_) {
        if (dynamic_cast<ActiveLayerChangedEvent*>(event.get())) {
            foundActiveChangeEvent = true;
            break;
        }
    }
    EXPECT_TRUE(foundActiveChangeEvent);
}

// Performance and stress tests
TEST_F(LayerSystemTest, LayerManagerPerformance) {
    const int NUM_LAYERS = 100;
    
    // Test creating many layers
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_LAYERS; ++i) {
        layerManager_->createLayer("Performance Layer " + std::to_string(i));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 1000); // Should complete in less than 1 second
    EXPECT_EQ(layerManager_->getLayerCount(), NUM_LAYERS + 1); // +1 for default layer
    
    // Test accessing layers
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < NUM_LAYERS; ++i) {
        const Layer* layer = layerManager_->getLayer(i);
        EXPECT_NE(layer, nullptr);
    }
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_LT(duration.count(), 100); // Should be very fast
}

} // namespace EpiGimp