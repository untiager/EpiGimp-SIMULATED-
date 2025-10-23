//Toolbar core functionality
#include "../../include/UI/Toolbar.hpp"
#include <iostream>

namespace EpiGimp {

Toolbar::Toolbar(Rectangle bounds, EventDispatcher* dispatcher) 
    : bounds_(bounds), eventDispatcher_(dispatcher), currentTool_(DrawingTool::None)
{
    
    if (!dispatcher)
        throw std::invalid_argument("EventDispatcher cannot be null");

    // Subscribe to tool selection events
    dispatcher->subscribe<ToolSelectedEvent>([this](const ToolSelectedEvent& event) {
        setSelectedTool(event.toolType);
    });

    const float paletteWidth = 8 * (20 + 2) + 2 * 5; // 8 colors per row + padding = 186 pixels
    const float paletteHeight = bounds.height - 10; // Leave some padding
    const float paletteX = bounds.x + bounds.width - paletteWidth - 10; // Right side with margin
    const float paletteY = bounds.y + 5; // Small top margin
    
    Rectangle paletteRect = {paletteX, paletteY, paletteWidth, paletteHeight};
    colorPalette_ = std::make_unique<ColorPalette>(paletteRect, dispatcher);
    
    std::cout << "Toolbar initialized with bounds: " 
              << bounds.x << ", " << bounds.y << ", " 
              << bounds.width << ", " << bounds.height << std::endl;
}

void Toolbar::update(float deltaTime)
{
    for (auto& button : buttons_) {
        updateButton(*button);
    }
    
    if (colorPalette_)
        colorPalette_->update(deltaTime);
}

void Toolbar::draw() const
{
    DrawRectangleRec(bounds_, RAYWHITE);
    DrawRectangleLinesEx(bounds_, 1, LIGHTGRAY);
    
    for (const auto& button : buttons_) {
        drawButton(*button);
    }
    
    if (colorPalette_)
        colorPalette_->draw();
}

void Toolbar::addButton(const std::string& text, std::function<void()> onClick)
{
    const auto buttonBounds = calculateNextButtonBounds();
    auto button = std::make_unique<Button>(buttonBounds, text, std::move(onClick));
    buttons_.push_back(std::move(button));
    
    std::cout << "Added button: " << text << std::endl;
}

void Toolbar::setSelectedTool(DrawingTool tool)
{
    currentTool_ = tool;
    
    for (auto& button : buttons_) {
        // Set selection based on tool type
        if (button->text == "Crayon") {
            button->isSelected = (tool == DrawingTool::Crayon);
        } else if (button->text == "Brush") {
            button->isSelected = (tool == DrawingTool::Brush);
        } else if (button->text == "Select") {
            button->isSelected = (tool == DrawingTool::Select);
        } else if (button->text == "Mirror") {
            button->isSelected = (tool == DrawingTool::Mirror);
        } else if (button->text == "Eyedropper") {
            button->isSelected = (tool == DrawingTool::Eyedropper);
        } else {
            button->isSelected = false;
        }
    }
    
    std::string toolName = "None";
    if (tool == DrawingTool::Crayon) toolName = "Crayon";
    else if (tool == DrawingTool::Brush) toolName = "Brush";
    else if (tool == DrawingTool::Select) toolName = "Select";
    else if (tool == DrawingTool::Mirror) toolName = "Mirror";
    else if (tool == DrawingTool::Eyedropper) toolName = "Eyedropper";
    
    std::cout << "Tool selected: " << toolName << std::endl;
}

} // namespace EpiGimp