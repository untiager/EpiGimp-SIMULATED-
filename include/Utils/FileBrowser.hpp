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
    std::vector<std::string> tempExtensions_; // Temporary storage for extension filtering
    int selectedIndex_;
    bool showHidden_;
    std::string inputBuffer_;
    std::string saveFileName_;  // Store the filename for save operations
    bool cancelled_;
    
    double lastNavigationTime_;
    static constexpr double CLICK_DELAY_THRESHOLD = 0.3; // 300ms delay
    
    double lastBackspaceTime_;
    static constexpr double BACKSPACE_DELAY_THRESHOLD = 0.1; // 100ms delay
    
    void loadDirectory();
    bool hasValidExtension(const std::string& filename) const;
    bool canProcessClicks() const;
    bool canProcessBackspace() const;
    
public:
    FileBrowser();
    
    void setPath(const std::string& path);
    void goUp();
    void enterDirectory(const std::string& dirname);
    
    void setSupportedExtensions(const std::vector<std::string>& extensions);
    void setShowHidden(bool show);
    void setShowAllFiles(bool showAll); // Temporarily show all files for save dialog
    
    bool renderOpenDialog(float x, float y, float width, float height);
    bool renderSaveDialog(float x, float y, float width, float height);
    
    std::string getCurrentPath() const { return currentPath_; }
    std::optional<std::string> getSelectedFile() const;
    std::string getSaveFileName() const;
    
    void reset();
    bool isValidSelection() const;
    bool wasCancelled() const;
};

} // namespace EpiGimp

#endif // FILE_BROWSER_HPP