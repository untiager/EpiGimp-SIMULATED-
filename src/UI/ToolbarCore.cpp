//Toolbar core functionality
#include "../../include/UI/Toolbar.hpp"
#include <iostream>

namespace EpiGimp {

Toolbar::Toolbar(Rectangle bounds, EventDispatcher* dispatcher) 
    : bounds_(bounds), eventDispatcher_(dispatcher), currentTool_(DrawingTool::None), dropdownCloseCooldown_(0.0f), consumedClickThisFrame_(false)
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
    // Reset click consumption flag at start of frame
    consumedClickThisFrame_ = false;
    
    // Decrease cooldown timer
    if (dropdownCloseCooldown_ > 0.0f) {
        dropdownCloseCooldown_ -= deltaTime;
        if (dropdownCloseCooldown_ < 0.0f) {
            dropdownCloseCooldown_ = 0.0f;
        }
    }
    
    // Track if any dropdown consumed a click this frame
    bool dropdownConsumedClick = false;
    
    // Update dropdown menus first (they have priority)
    for (auto& menu : dropdownMenus_) {
        // Check if this dropdown will consume the click
        const Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            // Check if clicking on dropdown button
            if (CheckCollisionPointRec(mousePos, menu->bounds)) {
                dropdownConsumedClick = true;
                consumedClickThisFrame_ = true;
            }
            // Check if clicking on any menu item
            if (menu->isOpen) {
                for (const auto& item : menu->items) {
                    if (CheckCollisionPointRec(mousePos, item->bounds)) {
                        dropdownConsumedClick = true;
                        consumedClickThisFrame_ = true;
                        break;
                    }
                }
            }
        }
        
        updateDropdownMenu(*menu);
    }
    
    // Check if any dropdown is open
    bool anyDropdownOpen = false;
    for (const auto& menu : dropdownMenus_) {
        if (menu->isOpen) {
            anyDropdownOpen = true;
            break;
        }
    }
    
    // Only update buttons if:
    // - No dropdown is open
    // - Cooldown has expired
    // - No dropdown consumed a click this frame
    if (!anyDropdownOpen && dropdownCloseCooldown_ <= 0.0f && !dropdownConsumedClick) {
        for (auto& button : buttons_) {
            updateButton(*button);
        }
    } else {
        // Clear hover and pressed states when dropdowns are open or consumed click
        for (auto& button : buttons_) {
            button->isHovered = false;
            button->isPressed = false;
        }
    }
    
    if (colorPalette_)
        colorPalette_->update(deltaTime);
}

void Toolbar::draw() const
{
    DrawRectangleRec(bounds_, RAYWHITE);
    DrawRectangleLinesEx(bounds_, 1, LIGHTGRAY);
    
    // Draw dropdown menus
    for (const auto& menu : dropdownMenus_) {
        drawDropdownMenu(*menu);
    }
    
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
        } else if (button->text == "Airbrush") {
            button->isSelected = (tool == DrawingTool::Airbrush);
        } else if (button->text == "Ink") {
            button->isSelected = (tool == DrawingTool::Ink);
        } else if (button->text == "Select") {
            button->isSelected = (tool == DrawingTool::Select);
        } else if (button->text == "Mirror") {
            button->isSelected = (tool == DrawingTool::Mirror);
        } else if (button->text == "Eyedropper") {
            button->isSelected = (tool == DrawingTool::Eyedropper);
        } else if (button->text == "Blur") {
            button->isSelected = (tool == DrawingTool::Blur);
        } else {
            button->isSelected = false;
        }
    }
    
    std::string toolName = "None";
    if (tool == DrawingTool::Crayon) toolName = "Crayon";
    else if (tool == DrawingTool::Brush) toolName = "Brush";
    else if (tool == DrawingTool::Airbrush) toolName = "Airbrush";
    else if (tool == DrawingTool::Ink) toolName = "Ink";
    else if (tool == DrawingTool::Select) toolName = "Select";
    else if (tool == DrawingTool::Mirror) toolName = "Mirror";
    else if (tool == DrawingTool::Eyedropper) toolName = "Eyedropper";
    else if (tool == DrawingTool::Blur) toolName = "Blur";
    
    std::cout << "Tool selected: " << toolName << std::endl;
}

void Toolbar::addDropdownMenu(const std::string& label)
{
    const auto menuBounds = calculateNextDropdownBounds();
    auto menu = std::make_unique<DropdownMenu>(label);
    menu->bounds = menuBounds;
    dropdownMenus_.push_back(std::move(menu));
    
    std::cout << "Added dropdown menu: " << label << std::endl;
}

void Toolbar::addMenuItemToLastDropdown(const std::string& text, std::function<void()> onClick)
{
    if (dropdownMenus_.empty()) {
        std::cerr << "Cannot add menu item: no dropdown menu exists" << std::endl;
        return;
    }
    
    auto& lastMenu = dropdownMenus_.back();
    auto item = std::make_unique<MenuItem>(text, std::move(onClick));
    lastMenu->items.push_back(std::move(item));
    
    std::cout << "Added menu item: " << text << " to " << lastMenu->label << std::endl;
}

void Toolbar::updateDropdownMenu(DropdownMenu& menu)
{
    const Vector2 mousePos = GetMousePosition();
    
    menu.isHovered = CheckCollisionPointRec(mousePos, menu.bounds);
    
    // Toggle dropdown on click
    if (menu.isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        menu.isOpen = !menu.isOpen;
        
        // Close other menus
        for (auto& otherMenu : dropdownMenus_) {
            if (otherMenu.get() != &menu) {
                otherMenu->isOpen = false;
            }
        }
    }
    
    // Close menu if clicking outside
    if (menu.isOpen && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !menu.isHovered) {
        bool clickedOnItem = false;
        for (const auto& item : menu.items) {
            if (CheckCollisionPointRec(mousePos, item->bounds)) {
                clickedOnItem = true;
                break;
            }
        }
        if (!clickedOnItem) {
            menu.isOpen = false;
            dropdownCloseCooldown_ = DROPDOWN_CLOSE_COOLDOWN; // Set cooldown when closing
        }
    }
    
    // Update menu items
    if (menu.isOpen) {
        for (auto& item : menu.items) {
            item->isHovered = CheckCollisionPointRec(mousePos, item->bounds);
            
            if (item->isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                std::cout << "Menu item clicked: " << item->text << " - setting cooldown FIRST" << std::endl;
                
                // Set cooldown BEFORE executing callback to prevent any race conditions
                dropdownCloseCooldown_ = DROPDOWN_CLOSE_COOLDOWN;
                menu.isOpen = false;
                
                std::cout << "Cooldown set to: " << dropdownCloseCooldown_ << " before callback" << std::endl;
                
                // Now execute the callback
                if (item->onClick) {
                    item->onClick();
                }
            }
        }
    }
}

void Toolbar::drawDropdownMenu(const DropdownMenu& menu) const
{
    // Draw menu button
    Color buttonColor = menu.isHovered ? GRAY : LIGHTGRAY;
    if (menu.isOpen) {
        buttonColor = DARKGRAY;
    }
    
    DrawRectangleRec(menu.bounds, buttonColor);
    DrawRectangleLinesEx(menu.bounds, 1, DARKGRAY);
    
    // Draw menu label with dropdown arrow
    const int textWidth = MeasureText(menu.label.c_str(), FONT_SIZE);
    const int textX = static_cast<int>(menu.bounds.x + (menu.bounds.width - textWidth - 15) / 2);
    const int textY = static_cast<int>(menu.bounds.y + (menu.bounds.height - FONT_SIZE) / 2);
    
    DrawText(menu.label.c_str(), textX, textY, FONT_SIZE, BLACK);
    DrawText("v", textX + textWidth + 5, textY, FONT_SIZE, BLACK);
    
    // Draw dropdown items if open
    if (menu.isOpen && !menu.items.empty()) {
        // Calculate dropdown panel bounds
        const float panelWidth = menu.bounds.width;
        const float panelHeight = static_cast<float>(menu.items.size() * MENU_ITEM_HEIGHT);
        const Rectangle panelBounds = {
            menu.bounds.x,
            menu.bounds.y + menu.bounds.height,
            panelWidth,
            panelHeight
        };
        
        // Draw panel background
        DrawRectangleRec(panelBounds, RAYWHITE);
        DrawRectangleLinesEx(panelBounds, 1, DARKGRAY);
        
        // Draw menu items
        for (size_t i = 0; i < menu.items.size(); ++i) {
            auto& item = menu.items[i];
            
            // Update item bounds
            item->bounds = {
                panelBounds.x,
                panelBounds.y + static_cast<float>(i * MENU_ITEM_HEIGHT),
                panelWidth,
                static_cast<float>(MENU_ITEM_HEIGHT)
            };
            
            // Draw item background
            Color itemColor = item->isHovered ? LIGHTGRAY : RAYWHITE;
            DrawRectangleRec(item->bounds, itemColor);
            DrawRectangleLinesEx(item->bounds, 1, GRAY);
            
            // Draw item text
            const int itemTextX = static_cast<int>(item->bounds.x + 10);
            const int itemTextY = static_cast<int>(item->bounds.y + (MENU_ITEM_HEIGHT - FONT_SIZE) / 2);
            DrawText(item->text.c_str(), itemTextX, itemTextY, FONT_SIZE, BLACK);
        }
    }
}

Rectangle Toolbar::calculateNextDropdownBounds() const
{
    const float startX = bounds_.x + BUTTON_MARGIN;
    const float startY = bounds_.y + BUTTON_MARGIN;
    
    // Calculate position after existing dropdowns and buttons
    float nextX = startX;
    
    // Account for dropdown menus
    for (const auto& menu : dropdownMenus_) {
        nextX = menu->bounds.x + menu->bounds.width + BUTTON_MARGIN;
    }
    
    return Rectangle{nextX, startY, BUTTON_WIDTH, BUTTON_HEIGHT};
}

} // namespace EpiGimp