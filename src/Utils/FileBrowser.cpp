//Safe in-application file browser implementation
#include "../../include/Utils/FileBrowser.hpp"
#include "raylib.h"
#include <algorithm>
#include <iostream>

namespace EpiGimp {

FileBrowser::FileBrowser() 
    : currentPath_(std::filesystem::current_path().string())
    , selectedIndex_(-1)
    , showHidden_(false)
    , inputBuffer_("")
{
    // Default supported image extensions
    supportedExtensions_ = {".png", ".jpg", ".jpeg", ".bmp", ".gif", ".tga", ".tiff"};
    inputBuffer_.resize(256); // Pre-allocate buffer for filename input
    loadDirectory();
}

void FileBrowser::setPath(const std::string& path) {
    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
        currentPath_ = path;
        selectedIndex_ = -1;
        loadDirectory();
    }
}

void FileBrowser::goUp() {
    std::filesystem::path currentDir(currentPath_);
    if (currentDir.has_parent_path()) {
        setPath(currentDir.parent_path().string());
    }
}

void FileBrowser::enterDirectory(const std::string& dirname) {
    std::filesystem::path newPath = std::filesystem::path(currentPath_) / dirname;
    setPath(newPath.string());
}

void FileBrowser::setSupportedExtensions(const std::vector<std::string>& extensions) {
    supportedExtensions_ = extensions;
    loadDirectory();
}

void FileBrowser::setShowHidden(bool show) {
    showHidden_ = show;
    loadDirectory();
}

void FileBrowser::loadDirectory() {
    entries_.clear();
    
    try {
        // Add parent directory entry if not at root
        std::filesystem::path currentDir(currentPath_);
        if (currentDir.has_parent_path()) {
            entries_.push_back({"..", currentDir.parent_path().string(), true, 0});
        }
        
        // Load directory contents
        for (const auto& entry : std::filesystem::directory_iterator(currentPath_)) {
            std::string filename = entry.path().filename().string();
            
            // Skip hidden files unless requested
            if (!showHidden_ && filename[0] == '.') {
                continue;
            }
            
            bool isDir = entry.is_directory();
            size_t size = 0;
            
            if (!isDir) {
                // For files, check if it's a supported type
                if (!hasValidExtension(filename)) {
                    continue;
                }
                
                try {
                    size = entry.file_size();
                } catch (...) {
                    size = 0;
                }
            }
            
            entries_.push_back({
                filename,
                entry.path().string(),
                isDir,
                size
            });
        }
        
        // Sort entries: directories first, then files
        std::sort(entries_.begin(), entries_.end(), [](const FileEntry& a, const FileEntry& b) {
            if (a.name == "..") return true;
            if (b.name == "..") return false;
            if (a.isDirectory != b.isDirectory) {
                return a.isDirectory;
            }
            return a.name < b.name;
        });
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading directory: " << e.what() << std::endl;
    }
}

bool FileBrowser::hasValidExtension(const std::string& filename) const {
    if (supportedExtensions_.empty()) {
        return true; // No filter, show all files
    }
    
    std::string lowerFilename = filename;
    std::transform(lowerFilename.begin(), lowerFilename.end(), lowerFilename.begin(), ::tolower);
    
    for (const auto& ext : supportedExtensions_) {
        std::string lowerExt = ext;
        std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
        
        if (lowerFilename.size() >= lowerExt.size() &&
            lowerFilename.substr(lowerFilename.size() - lowerExt.size()) == lowerExt) {
            return true;
        }
    }
    
    return false;
}

// Simple button helper
bool drawButton(Rectangle bounds, const char* text, bool isSelected = false) {
    Color bgColor = isSelected ? LIGHTGRAY : GRAY;
    Color textColor = isSelected ? BLACK : WHITE;
    
    DrawRectangleRec(bounds, bgColor);
    DrawRectangleLinesEx(bounds, 1, DARKGRAY);
    
    Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 14, 1);
    Vector2 textPos = {
        bounds.x + (bounds.width - textSize.x) / 2,
        bounds.y + (bounds.height - textSize.y) / 2
    };
    
    DrawText(text, (int)textPos.x, (int)textPos.y, 14, textColor);
    
    Vector2 mousePos = GetMousePosition();
    return CheckCollisionPointRec(mousePos, bounds) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
}

bool FileBrowser::renderOpenDialog(float x, float y, float width, float height) {
    // Draw background panel
    DrawRectangle((int)x, (int)y, (int)width, (int)height, RAYWHITE);
    DrawRectangleLinesEx(Rectangle{x, y, width, height}, 2, BLACK);
    
    // Title
    DrawText("Open Image File", (int)(x + 10), (int)(y + 10), 16, BLACK);
    
    float padding = 10;
    float buttonHeight = 30;
    float listHeight = height - 120; // Leave space for buttons and path
    
    // Current path display
    DrawText(currentPath_.c_str(), (int)(x + padding), (int)(y + 35), 12, DARKGRAY);
    
    // Up button
    Rectangle upButton = {x + width - 80, y + 30, 70, 25};
    if (drawButton(upButton, "Up")) {
        goUp();
    }
    
    // File list
    float itemHeight = 25;
    float listY = y + 65;
    int visibleItems = (int)(listHeight / itemHeight);
    
    for (int i = 0; i < (int)entries_.size() && i < visibleItems; i++) {
        Rectangle itemRect = {x + padding, listY + i * itemHeight, width - 2*padding, itemHeight - 2};
        
        bool isSelected = (selectedIndex_ == i);
        
        // Highlight selected item
        if (isSelected) {
            DrawRectangleRec(itemRect, SKYBLUE);
        }
        
        // Draw item
        std::string displayText = entries_[i].isDirectory ? 
            "[DIR] " + entries_[i].name : entries_[i].name;
            
        if (drawButton(itemRect, displayText.c_str(), isSelected)) {
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
    
    // Control buttons
    float buttonY = y + height - 40;
    Rectangle openBtn = {x + padding, buttonY, 80, buttonHeight};
    Rectangle cancelBtn = {x + width - 90, buttonY, 80, buttonHeight};
    
    bool openClicked = drawButton(openBtn, "Open");
    bool cancelClicked = drawButton(cancelBtn, "Cancel");
    
    if (openClicked && isValidSelection()) {
        return true; // File selected
    }
    
    if (cancelClicked || IsKeyPressed(KEY_ESCAPE)) {
        selectedIndex_ = -1;
        return false; // Cancelled
    }
    
    return false; // Continue showing dialog
}

bool FileBrowser::renderSaveDialog(float x, float y, float width, float height) {
    // Draw background panel
    DrawRectangle((int)x, (int)y, (int)width, (int)height, RAYWHITE);
    DrawRectangleLinesEx(Rectangle{x, y, width, height}, 2, BLACK);
    
    // Title
    DrawText("Save Image As", (int)(x + 10), (int)(y + 10), 16, BLACK);
    
    float padding = 10;
    float buttonHeight = 30;
    float listHeight = height - 160; // Leave more space for filename input
    
    // Current path display
    DrawText(currentPath_.c_str(), (int)(x + padding), (int)(y + 35), 12, DARKGRAY);
    
    // Up button
    Rectangle upButton = {x + width - 80, y + 30, 70, 25};
    if (drawButton(upButton, "Up")) {
        goUp();
    }
    
    // Directory list (only directories and for reference)
    float itemHeight = 25;
    float listY = y + 65;
    int visibleItems = (int)(listHeight / itemHeight);
    
    for (int i = 0; i < (int)entries_.size() && i < visibleItems; i++) {
        if (!entries_[i].isDirectory) continue; // Only show directories in save dialog
        
        Rectangle itemRect = {x + padding, listY + (i * itemHeight / 2), width - 2*padding, itemHeight - 2};
        
        std::string displayText = "[DIR] " + entries_[i].name;
        
        if (drawButton(itemRect, displayText.c_str())) {
            if (entries_[i].name == "..") {
                goUp();
            } else {
                enterDirectory(entries_[i].name);
            }
        }
    }
    
    // Filename input (simple text display for now)
    float inputY = y + height - 80;
    DrawText("Filename:", (int)(x + padding), (int)(inputY - 20), 14, BLACK);
    
    Rectangle inputRect = {x + padding, inputY, width - 2*padding - 200, 30};
    DrawRectangleRec(inputRect, WHITE);
    DrawRectangleLinesEx(inputRect, 1, GRAY);
    
    // Simple text input handling
    int key = GetCharPressed();
    while (key > 0) {
        if (key >= 32 && key <= 125 && inputBuffer_.size() < 250) {
            inputBuffer_ += (char)key;
        }
        key = GetCharPressed();
    }
    
    // Handle backspace
    if (IsKeyPressed(KEY_BACKSPACE) && !inputBuffer_.empty()) {
        inputBuffer_.pop_back();
    }
    
    DrawText(inputBuffer_.c_str(), (int)(inputRect.x + 5), (int)(inputRect.y + 8), 14, BLACK);
    
    // Control buttons
    float buttonY = y + height - 40;
    Rectangle saveBtn = {x + padding, buttonY, 80, buttonHeight};
    Rectangle cancelBtn = {x + width - 90, buttonY, 80, buttonHeight};
    
    bool saveClicked = drawButton(saveBtn, "Save");
    bool cancelClicked = drawButton(cancelBtn, "Cancel");
    
    if (saveClicked && !inputBuffer_.empty()) {
        return true; // Save with filename
    }
    
    if (cancelClicked || IsKeyPressed(KEY_ESCAPE)) {
        inputBuffer_.clear();
        return false; // Cancelled
    }
    
    return false; // Continue showing dialog
}

std::optional<std::string> FileBrowser::getSelectedFile() const {
    if (selectedIndex_ >= 0 && selectedIndex_ < (int)entries_.size() && 
        !entries_[selectedIndex_].isDirectory) {
        return entries_[selectedIndex_].fullPath;
    }
    return std::nullopt;
}

std::string FileBrowser::getSaveFileName() const {
    if (!inputBuffer_.empty()) {
        std::filesystem::path fullPath = std::filesystem::path(currentPath_) / inputBuffer_;
        return fullPath.string();
    }
    return "";
}

void FileBrowser::reset() {
    selectedIndex_ = -1;
    inputBuffer_.clear();
    setPath(std::filesystem::current_path().string());
}

bool FileBrowser::isValidSelection() const {
    return selectedIndex_ >= 0 && selectedIndex_ < (int)entries_.size() && 
           !entries_[selectedIndex_].isDirectory;
}

} // namespace EpiGimp