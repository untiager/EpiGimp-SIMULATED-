// Implementation of SimpleFileManager class
#include "../../include/Utils/Implementations.hpp"
#include "../../include/Utils/FileBrowser.hpp"
#include "raylib.h"

namespace EpiGimp {

SimpleFileManager::SimpleFileManager() 
    : showingOpenDialog_(false)
    , showingSaveDialog_(false) 
{
    openBrowser_ = std::make_unique<FileBrowser>();
    saveBrowser_ = std::make_unique<FileBrowser>();
}

SimpleFileManager::~SimpleFileManager() = default;

std::optional<std::string> SimpleFileManager::showOpenDialog(const std::string& /*filter*/)
{
    showingOpenDialog_ = true;
    openBrowser_->reset();
    openBrowser_->setShowAllFiles(true); // Show all files for consistency with save dialog
    return std::nullopt; // Will be handled in update loop
}

std::optional<std::string> SimpleFileManager::showSaveDialog(const std::string& /*filter*/, 
                                        const std::string& /*defaultName*/)
{
    showingSaveDialog_ = true;
    saveBrowser_->reset();
    saveBrowser_->setShowAllFiles(true); // Show all files for context in save dialog
    return std::nullopt; // Will be handled in update loop
}

std::optional<std::string> SimpleFileManager::updateOpenDialog()
{
    if (!showingOpenDialog_) return std::nullopt;
    
    float screenWidth = (float)GetScreenWidth();
    float screenHeight = (float)GetScreenHeight();
    float dialogWidth = screenWidth * 0.8f;
    float dialogHeight = screenHeight * 0.8f;
    float x = (screenWidth - dialogWidth) / 2;
    float y = (screenHeight - dialogHeight) / 2;
    
    // Semi-transparent background
    DrawRectangle(0, 0, (int)screenWidth, (int)screenHeight, ColorAlpha(BLACK, 0.7f));
    
    bool result = openBrowser_->renderOpenDialog(x, y, dialogWidth, dialogHeight);
    
    if (result) {
        showingOpenDialog_ = false;
        openBrowser_->setShowAllFiles(false); // Restore normal filtering
        return openBrowser_->getSelectedFile();
    }
    
    if (openBrowser_->wasCancelled()) {
        showingOpenDialog_ = false;
        openBrowser_->setShowAllFiles(false); // Restore normal filtering
        return std::nullopt;
    }
    
    return std::nullopt;
}

std::optional<std::string> SimpleFileManager::updateSaveDialog()
{
    if (!showingSaveDialog_) return std::nullopt;
    
    float screenWidth = (float)GetScreenWidth();
    float screenHeight = (float)GetScreenHeight();
    float dialogWidth = screenWidth * 0.8f;
    float dialogHeight = screenHeight * 0.8f;
    float x = (screenWidth - dialogWidth) / 2;
    float y = (screenHeight - dialogHeight) / 2;
    
    // Semi-transparent background
    DrawRectangle(0, 0, (int)screenWidth, (int)screenHeight, ColorAlpha(BLACK, 0.7f));
    
    bool result = saveBrowser_->renderSaveDialog(x, y, dialogWidth, dialogHeight);
    
    if (result) {
        showingSaveDialog_ = false;
        saveBrowser_->setShowAllFiles(false); // Restore normal filtering
        std::string filename = saveBrowser_->getSaveFileName();
        return filename.empty() ? std::nullopt : std::make_optional(filename);
    }
    
    // Check if dialog was cancelled (Cancel button or ESC key)
    if (saveBrowser_->wasCancelled()) {
        showingSaveDialog_ = false;
        saveBrowser_->setShowAllFiles(false); // Restore normal filtering
        return std::nullopt;
    }
    
    return std::nullopt;
}

bool SimpleFileManager::isShowingDialog() const
{
    return showingOpenDialog_ || showingSaveDialog_;
}

bool SimpleFileManager::fileExists(const std::string& path) const
{
    return std::filesystem::exists(path);
}

bool SimpleFileManager::createDirectories(const std::string& path) const
{
    std::error_code ec;
    return std::filesystem::create_directories(path, ec);
}

} // namespace EpiGimp