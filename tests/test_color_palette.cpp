#include <gtest/gtest.h>
#include "../include/UI/Toolbar.hpp"
#include "../include/Core/EventSystem.hpp"
#include "test_globals.hpp"

using namespace EpiGimp;

class ColorPaletteTest : public ::testing::Test {
protected:
    void SetUp() override {
        eventDispatcher_ = std::make_unique<EventDispatcher>();
        
        // Track color events
        primaryColorReceived_ = false;
        secondaryColorReceived_ = false;
        receivedPrimaryColor_ = BLACK;
        receivedSecondaryColor_ = BLACK;
        
        eventDispatcher_->subscribe<PrimaryColorChangedEvent>([this](const PrimaryColorChangedEvent& event) {
            primaryColorReceived_ = true;
            receivedPrimaryColor_ = event.primaryColor;
        });
        
        eventDispatcher_->subscribe<SecondaryColorChangedEvent>([this](const SecondaryColorChangedEvent& event) {
            secondaryColorReceived_ = true;
            receivedSecondaryColor_ = event.secondaryColor;
        });
        
        colorPalette_ = std::make_unique<ColorPalette>(
            Rectangle{0, 0, 200, 100}, 
            eventDispatcher_.get()
        );
    }
    
    void TearDown() override {
        colorPalette_.reset();
        eventDispatcher_.reset();
    }
    
    std::unique_ptr<EventDispatcher> eventDispatcher_;
    std::unique_ptr<ColorPalette> colorPalette_;
    bool primaryColorReceived_;
    bool secondaryColorReceived_;
    Color receivedPrimaryColor_;
    Color receivedSecondaryColor_;
};

TEST_F(ColorPaletteTest, InitializationWithPrimaryAndSecondaryColors) {
    // Test that ColorPalette initializes with default primary and secondary colors
    EXPECT_NE(colorPalette_, nullptr);
    
    // Default primary should be BLACK, secondary should be WHITE
    EXPECT_EQ(colorPalette_->getPrimaryColor().r, 0);
    EXPECT_EQ(colorPalette_->getPrimaryColor().g, 0);
    EXPECT_EQ(colorPalette_->getPrimaryColor().b, 0);
    
    EXPECT_EQ(colorPalette_->getSecondaryColor().r, 255);
    EXPECT_EQ(colorPalette_->getSecondaryColor().g, 255);
    EXPECT_EQ(colorPalette_->getSecondaryColor().b, 255);
}

TEST_F(ColorPaletteTest, PrimaryColorSetting) {
    // Test setting primary color programmatically
    Color testColor = RED;
    colorPalette_->setPrimaryColor(testColor);
    
    Color retrievedColor = colorPalette_->getPrimaryColor();
    EXPECT_EQ(retrievedColor.r, testColor.r);
    EXPECT_EQ(retrievedColor.g, testColor.g);
    EXPECT_EQ(retrievedColor.b, testColor.b);
}

TEST_F(ColorPaletteTest, SecondaryColorSetting) {
    // Test setting secondary color programmatically
    Color testColor = BLUE;
    colorPalette_->setSecondaryColor(testColor);
    
    Color retrievedColor = colorPalette_->getSecondaryColor();
    EXPECT_EQ(retrievedColor.r, testColor.r);
    EXPECT_EQ(retrievedColor.g, testColor.g);
    EXPECT_EQ(retrievedColor.b, testColor.b);
}

TEST_F(ColorPaletteTest, PrimaryColorChangeEvent) {
    // Test that setting primary color emits the correct event
    Color testColor = GREEN;
    
    eventDispatcher_->emit<PrimaryColorChangedEvent>(testColor);
    
    EXPECT_TRUE(primaryColorReceived_);
    EXPECT_EQ(receivedPrimaryColor_.r, testColor.r);
    EXPECT_EQ(receivedPrimaryColor_.g, testColor.g);
    EXPECT_EQ(receivedPrimaryColor_.b, testColor.b);
}

TEST_F(ColorPaletteTest, SecondaryColorChangeEvent) {
    // Test that setting secondary color emits the correct event
    Color testColor = YELLOW;
    
    eventDispatcher_->emit<SecondaryColorChangedEvent>(testColor);
    
    EXPECT_TRUE(secondaryColorReceived_);
    EXPECT_EQ(receivedSecondaryColor_.r, testColor.r);
    EXPECT_EQ(receivedSecondaryColor_.g, testColor.g);
    EXPECT_EQ(receivedSecondaryColor_.b, testColor.b);
}

TEST_F(ColorPaletteTest, BackwardCompatibilityWithSelectedColor) {
    // Test that the old getSelectedColor method still works
    Color testColor = PURPLE;
    colorPalette_->setSelectedColor(testColor);
    
    // Should set primary color and maintain backward compatibility
    Color selectedColor = colorPalette_->getSelectedColor();
    Color primaryColor = colorPalette_->getPrimaryColor();
    
    EXPECT_EQ(selectedColor.r, testColor.r);
    EXPECT_EQ(selectedColor.g, testColor.g);
    EXPECT_EQ(selectedColor.b, testColor.b);
    
    EXPECT_EQ(primaryColor.r, testColor.r);
    EXPECT_EQ(primaryColor.g, testColor.g);
    EXPECT_EQ(primaryColor.b, testColor.b);
}