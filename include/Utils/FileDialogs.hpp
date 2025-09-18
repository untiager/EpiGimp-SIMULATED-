#ifndef FILE_DIALOGS_HPP
#define FILE_DIALOGS_HPP

#include <optional>
#include <string>

namespace EpiGimp {
namespace FileDialogs {

/**
 * Show an open file dialog
 * @param filter File filter (e.g., "Images (*.png *.jpg)")
 * @return Selected file path or nullopt if cancelled
 */
std::optional<std::string> showOpenDialog(const std::string& filter = "");

/**
 * Show a save file dialog
 * @param filter File filter (e.g., "Images (*.png)")
 * @param defaultName Default filename
 * @return Selected file path or nullopt if cancelled
 */
std::optional<std::string> showSaveDialog(const std::string& filter = "", 
                                         const std::string& defaultName = "");

} // namespace FileDialogs
} // namespace EpiGimp

#endif // FILE_DIALOGS_HPP