//FileBrowser core functionality
#include "../../include/Utils/FileBrowser.hpp"
#include "raylib.h"
#include <algorithm>
#include <iostream>

namespace EpiGimp {

FileBrowser::FileBrowser() 
    : selectedIndex_(-1), showHidden_(false), cancelled_(false), lastNavigationTime_(0.0), lastBackspaceTime_(0.0)
{
    currentPath_ = std::filesystem::current_path().string();
    setSupportedExtensions({".png", ".jpg", ".jpeg", ".bmp", ".tga"});
    loadDirectory();
}

void FileBrowser::setSupportedExtensions(const std::vector<std::string>& extensions)
{
    supportedExtensions_ = extensions;
}

void FileBrowser::setShowAllFiles(bool showAll)
{
    if (showAll) {
        // Temporarily clear extensions to show all files
        tempExtensions_ = supportedExtensions_;
        supportedExtensions_.clear();
        loadDirectory(); // Reload to show all files
    } else {
        // Restore original extensions
        supportedExtensions_ = tempExtensions_;
        loadDirectory(); // Reload with filtering
    }
}

void FileBrowser::setShowHidden(bool show)
{
    showHidden_ = show;
}

bool FileBrowser::hasValidExtension(const std::string& filename) const
{
    if (supportedExtensions_.empty())
        return true; // No filter means all files are valid

    std::string extension = std::filesystem::path(filename).extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    
    for (const auto& ext : supportedExtensions_) {
        std::string lowerExt = ext;
        std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);
        if (extension == lowerExt)
            return true;
    }
    
    return false;
}

bool FileBrowser::canProcessClicks() const
{
    double currentTime = GetTime();
    return (currentTime - lastNavigationTime_) > CLICK_DELAY_THRESHOLD;
}

bool FileBrowser::canProcessBackspace() const
{
    double currentTime = GetTime();
    return (currentTime - lastBackspaceTime_) > BACKSPACE_DELAY_THRESHOLD;
}

std::optional<std::string> FileBrowser::getSelectedFile() const
{
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(entries_.size())) {
        auto fullPath = std::filesystem::path(currentPath_) / entries_[selectedIndex_].name;
        return fullPath.string();
    }
    return std::nullopt;
}

std::string FileBrowser::getSaveFileName() const
{
    std::string filename = saveFileName_;
    if (filename.empty())
        filename = "untitled";
    
    std::filesystem::path fullPath = std::filesystem::path(currentPath_) / filename;
    return fullPath.string();
}

void FileBrowser::reset()
{
    selectedIndex_ = -1;
    cancelled_ = false;
    saveFileName_.clear();
    lastNavigationTime_ = 0.0;
    lastBackspaceTime_ = 0.0;
}

bool FileBrowser::isValidSelection() const
{
    return selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(entries_.size());
}

bool FileBrowser::wasCancelled() const
{
    return cancelled_;
}

} // namespace EpiGimp