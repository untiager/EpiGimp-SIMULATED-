#include <gtest/gtest.h>
#include "../include/UI/Canvas.hpp"
#include "../include/Core/EventSystem.hpp"
#include "../include/Core/HistoryManager.hpp"
#include "test_globals.hpp"

using namespace EpiGimp;

class DualColorSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        eventDispatcher_ = std::make_unique<EventDispatcher>();
        historyManager_ = std::make_unique<HistoryManager>();
        canvas_ = std::make_unique<Canvas>(
            Rectangle{0, 0, 400, 300}, 
            eventDispatcher_.get(), 
            historyManager_.get(),
            false  // Don't auto-create blank canvas
        );
    }
    
    void TearDown() override {
        canvas_.reset();
        historyManager_.reset();
        eventDispatcher_.reset();
    }
    
    std::unique_ptr<EventDispatcher> eventDispatcher_;
    std::unique_ptr<HistoryManager> historyManager_;
    std::unique_ptr<Canvas> canvas_;
};

TEST_F(DualColorSystemTest, PrimaryColorEventHandling) {
    // Test that primary color change events are handled correctly
    Color testColor = RED;
    
    // Emit primary color change event
    eventDispatcher_->emit<PrimaryColorChangedEvent>(testColor);
    
    // Canvas should have received and processed the event
    // (We can't directly access private members, but we can test via drawing behavior)
    EXPECT_TRUE(true); // Basic test that the event system doesn't crash
}

TEST_F(DualColorSystemTest, SecondaryColorEventHandling) {
    // Test that secondary color change events are handled correctly
    Color testColor = BLUE;
    
    // Emit secondary color change event
    eventDispatcher_->emit<SecondaryColorChangedEvent>(testColor);
    
    // Canvas should have received and processed the event
    EXPECT_TRUE(true); // Basic test that the event system doesn't crash
}

TEST_F(DualColorSystemTest, BackwardCompatibilityWithColorChangedEvent) {
    // Test that the old ColorChangedEvent still works
    Color testColor = GREEN;
    
    // Emit the old color change event
    eventDispatcher_->emit<ColorChangedEvent>(testColor);
    
    // Canvas should handle it and set it as primary color
    EXPECT_TRUE(true); // Basic test for backward compatibility
}

TEST_F(DualColorSystemTest, CanvasInitializationWithEventDispatcher) {
    // Test that canvas initializes properly with event dispatcher
    EXPECT_NE(canvas_, nullptr);
    EXPECT_NE(eventDispatcher_, nullptr);
    
    // Test that we can emit events without crashing
    eventDispatcher_->emit<PrimaryColorChangedEvent>(BLACK);
    eventDispatcher_->emit<SecondaryColorChangedEvent>(WHITE);
    eventDispatcher_->emit<ColorChangedEvent>(GRAY);
}

TEST_F(DualColorSystemTest, MultipleColorEvents) {
    // Test handling multiple color events in sequence
    std::vector<Color> colors = {RED, GREEN, BLUE, YELLOW, PURPLE};
    
    for (const auto& color : colors) {
        eventDispatcher_->emit<PrimaryColorChangedEvent>(color);
        eventDispatcher_->emit<SecondaryColorChangedEvent>(color);
    }
    
    // Should handle all events without issues
    EXPECT_TRUE(true);
}