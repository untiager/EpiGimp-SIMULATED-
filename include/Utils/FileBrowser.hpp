//Safe in-application file browser interface
#ifndef FILE_BROWSER_HPP
#define FILE_BROWSER_HPP

#include <string>
#include <vector>
#include <filesystem>
#include <optional>

namespace EpiGimp {

struct FileEntry {
    std::string name;
    std::string fullPath;
    bool isDirectory;
    size_t size;
};

class FileBrowser {
private:
    std::string currentPath_;
    std::vector<FileEntry> entries_;
    std::vector<std::string> supportedExtensions_;
    int selectedIndex_;
    bool showHidden_;
    std::string inputBuffer_;
    bool cancelled_;
    
    // Click delay mechanism to prevent accidental double-clicks after navigation
    double lastNavigationTime_;
    static constexpr double NAVIGATION_DELAY = 0.3; // 300ms delay
    
    // Backspace debouncing to prevent double deletion
    double lastBackspaceTime_;
    static constexpr double BACKSPACE_DELAY = 0.1; // 100ms delay
    
    void loadDirectory();
    bool hasValidExtension(const std::string& filename) const;
    bool canProcessClicks() const;
    bool canProcessBackspace() const;
    
public:
    FileBrowser();
    
    // Navigation
    void setPath(const std::string& path);
    void goUp();
    void enterDirectory(const std::string& dirname);
    
    // File filtering
    void setSupportedExtensions(const std::vector<std::string>& extensions);
    void setShowHidden(bool show);
    
    // UI rendering
    bool renderOpenDialog(float x, float y, float width, float height);
    bool renderSaveDialog(float x, float y, float width, float height);
    
    // Results
    std::string getCurrentPath() const { return currentPath_; }
    std::optional<std::string> getSelectedFile() const;
    std::string getSaveFileName() const;
    
    // State
    void reset();
    bool isValidSelection() const;
    bool wasCancelled() const;
};

} // namespace EpiGimp

#endif // FILE_BROWSER_HPP