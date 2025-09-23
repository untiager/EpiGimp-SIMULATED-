//FileBrowser navigation functionality
#include "../../include/Utils/FileBrowser.hpp"
#include "raylib.h"
#include <algorithm>
#include <iostream>

namespace EpiGimp {

void FileBrowser::setPath(const std::string& path)
{
    if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
        currentPath_ = path;
        selectedIndex_ = -1;
        lastNavigationTime_ = GetTime(); // Set delay after navigation
        loadDirectory();
    }
}

void FileBrowser::goUp()
{
    std::filesystem::path currentDir(currentPath_);
    if (currentDir.has_parent_path()) {
        lastNavigationTime_ = GetTime(); // Set delay after navigation
        setPath(currentDir.parent_path().string());
    }
}

void FileBrowser::enterDirectory(const std::string& dirname)
{
    std::filesystem::path newPath = std::filesystem::path(currentPath_) / dirname;
    lastNavigationTime_ = GetTime(); // Set delay after navigation
    setPath(newPath.string());
}

void FileBrowser::loadDirectory()
{
    entries_.clear();
    
    try {
        // Add parent directory entry if not at root
        std::filesystem::path currentDir(currentPath_);
        if (currentDir.has_parent_path())
            entries_.push_back({"..", currentDir.parent_path().string(), true, 0});
        
        // Load directory contents
        for (const auto& entry : std::filesystem::directory_iterator(currentPath_)) {
            std::string filename = entry.path().filename().string();
            
            // Skip hidden files unless requested
            if (!showHidden_ && filename[0] == '.')
                continue;
            
            bool isDir = entry.is_directory();
            size_t size = 0;
            
            if (!isDir) {
                // For files, check if it's a supported type (unless showing all files)
                if (!supportedExtensions_.empty() && !hasValidExtension(filename))
                    continue;
                
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

} // namespace EpiGimp