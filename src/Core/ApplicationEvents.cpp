//Application event handling and component creation
#include "../../include/Core/Application.hpp"
#include "../../include/UI/Toolbar.hpp"
#include "../../include/UI/Canvas.hpp"
#include "../../include/UI/SimpleLayerPanel.hpp"
#include "../../include/Utils/Implementations.hpp"
#include "../../include/Commands/ClearCommand.hpp"
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
    
    eventDispatcher_->subscribe<ClearCanvasRequestEvent>([this](const ClearCanvasRequestEvent&) {
        onClearCanvasRequest();
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
    
    toolbar_->addButton("Brush", [this]() {
        eventDispatcher_->emit<ToolSelectedEvent>(DrawingTool::Brush);
    });
    
    toolbar_->addButton("Select", [this]() {
        eventDispatcher_->emit<ToolSelectedEvent>(DrawingTool::Select);
    });
    
    toolbar_->addButton("Clear", [this]() {
        eventDispatcher_->emit<ClearCanvasRequestEvent>();
    });
    
    toolbar_->addButton("All V", [this]() {
        if (canvas_) {
            static_cast<Canvas*>(canvas_.get())->flipCanvasVertical();
        }
    });
    
    toolbar_->addButton("All H", [this]() {
        if (canvas_) {
            static_cast<Canvas*>(canvas_.get())->flipCanvasHorizontal();
        }
    });
    
    // Create canvas (below toolbar, right of layer panel)
    const Rectangle canvasBounds = {
        330, static_cast<float>(toolbar_->getHeight()), // Leave space for layer panel
        static_cast<float>(config_.windowWidth - 330), // Reduced width for layer panel
        static_cast<float>(config_.windowHeight - toolbar_->getHeight() - 25) // Leave space for status bar
    };
    canvas_ = std::make_unique<Canvas>(canvasBounds, eventDispatcher_.get(), historyManager_.get());
    
    // Create layer panel on the left
    const Rectangle layerPanelBounds = {
        0,
        static_cast<float>(toolbar_->getHeight()),
        330,
        static_cast<float>(config_.windowHeight - toolbar_->getHeight() - 25)
    };
    layerPanel_ = std::make_unique<SimpleLayerPanel>(layerPanelBounds, 
                                                     static_cast<Canvas*>(canvas_.get()), 
                                                     eventDispatcher_.get());
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

void Application::onClearCanvasRequest()
{
    if (!canvas_ || !historyManager_) {
        std::cout << "Cannot clear canvas: missing canvas or history manager" << std::endl;
        return;
    }
    
    // Cast to access Canvas methods
    auto* canvas = static_cast<Canvas*>(canvas_.get());
    
    if (!canvas->hasImage()) {
        std::cout << "No image loaded to clear" << std::endl;
        return;
    }
    
    // Create and execute a clear command
    auto clearCommand = createClearCommand(canvas);
    if (historyManager_->executeCommand(std::move(clearCommand))) {
        std::cout << "Drawing layer cleared and added to history" << std::endl;
    } else {
        std::cout << "Failed to clear drawing layer" << std::endl;
    }
}

} // namespace EpiGimp