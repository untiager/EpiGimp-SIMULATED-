//Application event handling and component creation
#include "../../include/Core/Application.hpp"
#include "../../include/UI/Toolbar.hpp" 
#include "../../include/UI/Canvas.hpp"
#include <iostream>

namespace EpiGimp {

void Application::setupEventHandlers()
{
    // Subscribe to events
    eventDispatcher_->subscribe<LoadImageRequestEvent>([this](const LoadImageRequestEvent&) {
        onLoadImageRequest();
    });
    
    eventDispatcher_->subscribe<ImageSaveRequestEvent>([this](const ImageSaveRequestEvent& event) {
        onImageSaveRequest(event);
    });
    
    eventDispatcher_->subscribe<ErrorEvent>([this](const ErrorEvent& event) {
        onError(event);
    });
    
    eventDispatcher_->subscribe<ImageLoadedEvent>([](const ImageLoadedEvent& event) {
        std::cout << "Image loaded event: " << event.filePath << std::endl;
    });
    
    eventDispatcher_->subscribe<ImageSavedEvent>([](const ImageSavedEvent& event) {
        std::cout << "Image saved event: " << event.filePath 
                  << " (success: " << event.success << ")" << std::endl;
    });
    
    eventDispatcher_->subscribe<ToolSelectedEvent>([this](const ToolSelectedEvent& event) {
        onToolSelected(event);
    });
    
    // Subscribe to tool selection events
    eventDispatcher_->subscribe<ToolSelectedEvent>([this](const ToolSelectedEvent& event) {
        onToolSelected(event);
    });
}

void Application::createComponents()
{
    // Create toolbar
    const Rectangle toolbarBounds = {0, 0, static_cast<float>(config_.windowWidth), 60};
    toolbar_ = std::make_unique<Toolbar>(toolbarBounds, eventDispatcher_.get());
    
    // Add toolbar buttons
    toolbar_->addButton("Load Image", [this]() {
        eventDispatcher_->emit<LoadImageRequestEvent>();
    });
    
    toolbar_->addButton("Save Image", [this]() {
        eventDispatcher_->emit<ImageSaveRequestEvent>("");
    });
    
    toolbar_->addButton("Crayon", [this]() {
        eventDispatcher_->emit<ToolSelectedEvent>(DrawingTool::Crayon);
    });
    
    // Create canvas (below toolbar)
    const Rectangle canvasBounds = {
        0, static_cast<float>(toolbar_->getHeight()), 
        static_cast<float>(config_.windowWidth), 
        static_cast<float>(config_.windowHeight - toolbar_->getHeight() - 25) // Leave space for status bar
    };
    canvas_ = std::make_unique<Canvas>(canvasBounds, eventDispatcher_.get());
}

void Application::onLoadImageRequest()
{
    fileManager_->showOpenDialog("Images (*.png *.jpg *.bmp)");
}

void Application::onImageSaveRequest(const ImageSaveRequestEvent& /*event*/)
{
    if (!canvas_->hasImage()) {
        errorHandler_->handleError("No image to save");
        return;
    }
    
    // Show save dialog - actual saving will happen in update loop
    fileManager_->showSaveDialog("Images (*.png)", "output.png");
}

void Application::onError(const ErrorEvent& event)
{
    errorHandler_->handleError(event.message);
}

void Application::onToolSelected(const ToolSelectedEvent& event)
{
    currentTool_ = event.toolType;
    std::cout << "Tool selected: " << static_cast<int>(currentTool_) << std::endl;
    
    // Pass the tool selection to the canvas
    if (canvas_) {
        // We'll need to cast to access the setDrawingTool method
        auto* canvas = static_cast<Canvas*>(canvas_.get());
        canvas->setDrawingTool(currentTool_);
    }
}

} // namespace EpiGimp