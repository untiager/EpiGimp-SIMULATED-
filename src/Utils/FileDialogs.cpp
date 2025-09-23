#include <cstdlib>
#include <string>
#include <optional>
#include <cstdio>
#include <memory>
#include <stdexcept>

namespace EpiGimp {
namespace FileDialogs {

std::string execCommand(const char* cmd)
{
    char buffer[128];
    std::string result = "";
    
    auto pipeDeleter = [](FILE* f) { if (f) pclose(f); };
    std::unique_ptr<FILE, decltype(pipeDeleter)> pipe(popen(cmd, "r"), pipeDeleter);
    
    if (!pipe)
        throw std::runtime_error("popen() failed!");
    while (fgets(buffer, sizeof buffer, pipe.get()) != nullptr) {
        result += buffer;
    }
    
    // Remove trailing newline
    if (!result.empty() && result.back() == '\n')
        result.pop_back();
    
    return result;
}

std::optional<std::string> showOpenDialog(const std::string& filter)
{
    try {
        // Use zenity for file dialog on Linux
        std::string cmd = "zenity --file-selection --title=\"Open Image File\"";
        
        // Add file filters
        if (!filter.empty()) {
            if (filter.find("png") != std::string::npos || 
                filter.find("jpg") != std::string::npos || 
                filter.find("bmp") != std::string::npos) {
                cmd += " --file-filter=\"Image files (*.png *.jpg *.jpeg *.bmp *.gif *.tga *.tiff)|*.png *.jpg *.jpeg *.bmp *.gif *.tga *.tiff\"";
                cmd += " --file-filter=\"All files|*\"";
            }
        }
        
        std::string result = execCommand(cmd.c_str());
        
        if (result.empty())
            return std::nullopt; // User cancelled
        
        return result;
        
    } catch (const std::exception& e) {
        // Fallback - return empty optional if dialog fails
        return std::nullopt;
    }
}

std::optional<std::string> showSaveDialog(const std::string& filter, const std::string& defaultName)
{
    try {
        // Use zenity for save dialog on Linux
        std::string cmd = "zenity --file-selection --save --confirm-overwrite --title=\"Save Image As\"";
        
        if (!defaultName.empty())
            cmd += " --filename=\"" + defaultName + "\"";
        
        // Add file filters
        if (!filter.empty()) {
            if (filter.find("png") != std::string::npos) {
                cmd += " --file-filter=\"PNG files (*.png)|*.png\"";
                cmd += " --file-filter=\"All files|*\"";
            }
        }
        
        std::string result = execCommand(cmd.c_str());
        
        if (result.empty())
            return std::nullopt; // User cancelled
        
        // Ensure .png extension if not present
        if (result.find(".png") == std::string::npos && 
            result.find(".jpg") == std::string::npos && 
            result.find(".bmp") == std::string::npos) {
            result += ".png";
        }
        
        return result;
        
    } catch (const std::exception& e) {
        // Fallback - return empty optional if dialog fails
        return std::nullopt;
    }
}

} // namespace FileDialogs
} // namespace EpiGimp