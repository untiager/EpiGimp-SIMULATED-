//Modern Application class using RAII and dependency injection
#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <memory>
#include <string>
#include <vector>
#include "RaylibWrappers.hpp"
#include "EventSystem.hpp"
#include "Interfaces.hpp"
#include "HistoryManager.hpp"

namespace EpiGimp {

// Forward declarations
class SimpleLayerPanel;

// Application configuration
struct AppConfig {
    int windowWidth = 1920;
    int windowHeight = 1080;
    std::string windowTitle = "EpiGimp - Modern Paint Interface";
    int targetFPS = 60;
    std::string initialImagePath;
};

// Main application class
class Application {
private:
    std::unique_ptr<WindowResource> window_;
    std::unique_ptr<EventDispatcher> eventDispatcher_;
    std::unique_ptr<IToolbar> toolbar_;
    std::unique_ptr<ICanvas> canvas_;
    std::unique_ptr<IFileManager> fileManager_;
    std::unique_ptr<IErrorHandler> errorHandler_;
    std::unique_ptr<IInputHandler> inputHandler_;
    std::unique_ptr<HistoryManager> historyManager_;
    std::unique_ptr<SimpleLayerPanel> layerPanel_;
    
    AppConfig config_;
    bool running_;
    bool initialized_;
    DrawingTool currentTool_;  // Current drawing tool

public:
    explicit Application(AppConfig config = AppConfig{});
    ~Application(); // Custom destructor needed for unique_ptr with forward declarations

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = default;
    Application& operator=(Application&&) = default;

    bool initialize();
    void run();
    void shutdown();

    // Component access (for dependency injection)
    EventDispatcher& getEventDispatcher() { return *eventDispatcher_; }
    IToolbar& getToolbar() { return *toolbar_; }
    ICanvas& getCanvas() { return *canvas_; }
    IFileManager& getFileManager() { return *fileManager_; }
    IErrorHandler& getErrorHandler() { return *errorHandler_; }
    IInputHandler& getInputHandler() { return *inputHandler_; }
    HistoryManager& getHistoryManager() { return *historyManager_; }

private:
    void update(float deltaTime);
    void draw();
    void handleEvents();
    void setupEventHandlers();
    void createComponents();
    
    void onLoadImageRequest();
    void onImageSaveRequest(const ImageSaveRequestEvent& event);
    void onError(const ErrorEvent& event);
    void onToolSelected(const ToolSelectedEvent& event);
    void onClearCanvasRequest();
};

} // namespace EpiGimp

#endif // APPLICATION_HPP