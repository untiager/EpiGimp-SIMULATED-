//FileBrowser dialog rendering functionality
#include "../../include/Utils/FileBrowser.hpp"
#include "raylib.h"
#include <iostream>

namespace EpiGimp {

static bool drawButton(Rectangle bounds, const char* text, bool isSelected = false)
{
    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, bounds);
    
    Color bgColor;
    Color textColor;
    
    if (isSelected) {
        bgColor = SKYBLUE;
        textColor = WHITE;
    } else if (isHovered) {
        bgColor = LIGHTGRAY;
        textColor = BLACK;
    } else {
        bgColor = GRAY;
        textColor = WHITE;
    }
    
    DrawRectangleRec(bounds, bgColor);
    DrawRectangleLinesEx(bounds, 1, DARKGRAY);
    
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 14, 1);
    Vector2 textPos = {
        bounds.x + (bounds.width - textSize.x) / 2,
        bounds.y + (bounds.height - textSize.y) / 2
    };
    
    DrawText(text, (int)textPos.x, (int)textPos.y, 14, textColor);
    
    return isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

bool FileBrowser::renderOpenDialog(float x, float y, float width, float height)
{
    DrawRectangle((int)x, (int)y, (int)width, (int)height, RAYWHITE);
    DrawRectangleLinesEx(Rectangle{x, y, width, height}, 2, BLACK);
    
    DrawText("Open Image File", (int)(x + 10), (int)(y + 10), 16, BLACK);
    
    float padding = 10;
    float buttonHeight = 30;
    float listHeight = height - 120;
    
    DrawText(currentPath_.c_str(), (int)(x + padding), (int)(y + 35), 12, DARKGRAY);
    
    Rectangle upButton = {x + width - 80, y + 30, 70, 25};
    if (drawButton(upButton, "Up") && canProcessClicks())
        goUp();
    
    float itemHeight = 25;
    float listY = y + 65;
    int visibleItems = (int)(listHeight / itemHeight);
    
    for (int i = 0; i < (int)entries_.size() && i < visibleItems; i++) {
        Rectangle itemRect = {x + padding, listY + i * itemHeight, width - 2*padding, itemHeight - 2};
        
        bool isSelected = (selectedIndex_ == i);
        
        // Draw item with proper selection and hover
        std::string displayText = entries_[i].isDirectory ? 
            "[DIR] " + entries_[i].name : entries_[i].name;
            
        if (drawButton(itemRect, displayText.c_str(), isSelected) && canProcessClicks()) {
            if (entries_[i].isDirectory) {
                if (entries_[i].name == "..") {
                    goUp();
                } else {
                    enterDirectory(entries_[i].name);
                }
            } else {
                selectedIndex_ = i;
            }
        }
    }
    
    float buttonY = y + height - 40;
    Rectangle openBtn = {x + padding, buttonY, 80, buttonHeight};
    Rectangle cancelBtn = {x + width - 90, buttonY, 80, buttonHeight};
    
    bool openClicked = drawButton(openBtn, "Open");
    bool cancelClicked = drawButton(cancelBtn, "Cancel");
    
    if (openClicked && isValidSelection()) {
        cancelled_ = false;
        return true; // File selected
    }
    
    if (cancelClicked || IsKeyPressed(KEY_ESCAPE)) {
        selectedIndex_ = -1;
        cancelled_ = true;
        return false; // Cancelled
    }
    
    cancelled_ = false;
    return false; // Continue showing dialog
}

bool FileBrowser::renderSaveDialog(float x, float y, float width, float height)
{
    DrawRectangle((int)x, (int)y, (int)width, (int)height, RAYWHITE);
    DrawRectangleLinesEx(Rectangle{x, y, width, height}, 2, BLACK);
    
    DrawText("Save Image As", (int)(x + 10), (int)(y + 10), 16, BLACK);
    
    float padding = 10;
    float buttonHeight = 30;
    float listHeight = height - 160; // Leave more space for filename input
    
    DrawText(currentPath_.c_str(), (int)(x + padding), (int)(y + 35), 12, DARKGRAY);
    
    Rectangle upButton = {x + width - 80, y + 30, 70, 25};
    if (drawButton(upButton, "Up") && canProcessClicks())
        goUp();

    float itemHeight = 25;
    float listY = y + 65;
    int visibleItems = (int)(listHeight / itemHeight);
    
    for (int i = 0; i < (int)entries_.size() && i < visibleItems; i++) {
        Rectangle itemRect = {x + padding, listY + (i * itemHeight), width - 2*padding, itemHeight - 2};
        
        std::string displayText;
        Color textColor = BLACK;
        
        if (entries_[i].isDirectory) {
            displayText = "[DIR] " + entries_[i].name;
            textColor = BLUE; // Highlight directories
        } else {
            displayText = entries_[i].name;
            textColor = DARKGRAY; // Files shown for reference only
        }
        
        // Only directories are clickable in save dialog
        if (entries_[i].isDirectory && drawButton(itemRect, displayText.c_str()) && canProcessClicks()) {
            if (entries_[i].name == "..") {
                goUp();
            } else {
                enterDirectory(entries_[i].name);
            }
        } else if (!entries_[i].isDirectory) {
            // Draw files as non-clickable for reference
            DrawRectangleRec(itemRect, LIGHTGRAY);
            DrawRectangleLinesEx(itemRect, 1, GRAY);
            DrawText(displayText.c_str(), (int)(itemRect.x + 5), (int)(itemRect.y + 5), 14, textColor);
        }
    }
    
    float inputY = y + height - 80;
    DrawText("Filename:", (int)(x + padding), (int)(inputY - 20), 14, BLACK);
    
    Rectangle inputRect = {x + padding, inputY, width - 2*padding - 200, 30};
    DrawRectangleRec(inputRect, WHITE);
    DrawRectangleLinesEx(inputRect, 1, GRAY);
    
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 125 && inputBuffer_.size() < 250)
            inputBuffer_ += (char)key;
        key = GetCharPressed();
    }
    
    if (IsKeyPressed(KEY_BACKSPACE) && !inputBuffer_.empty() && canProcessBackspace()) {
        inputBuffer_.pop_back();
        lastBackspaceTime_ = GetTime();
    }
    
    DrawText(inputBuffer_.c_str(), (int)(inputRect.x + 5), (int)(inputRect.y + 8), 14, BLACK);
    
    float helpY = inputY + 35;
    std::string helpText = "Supported formats: .png, .jpg, .jpeg, .bmp, .tga (auto-adds .png if missing)";
    DrawText(helpText.c_str(), (int)(x + padding), (int)(helpY), 12, DARKGRAY);
    
    float buttonY = y + height - 40;
    Rectangle saveBtn = {x + padding, buttonY, 80, buttonHeight};
    Rectangle cancelBtn = {x + width - 90, buttonY, 80, buttonHeight};
    
    bool saveClicked = drawButton(saveBtn, "Save");
    bool cancelClicked = drawButton(cancelBtn, "Cancel");
    
    if (saveClicked && !inputBuffer_.empty()) {
        cancelled_ = false;
        saveFileName_ = inputBuffer_; // Store the entered filename
        return true; // Save with filename
    }
    
    if (cancelClicked || IsKeyPressed(KEY_ESCAPE)) {
        inputBuffer_.clear();
        cancelled_ = true;
        return false; // Cancelled
    }
    
    cancelled_ = false;
    return false; // Continue showing dialog
}

} // namespace EpiGimp