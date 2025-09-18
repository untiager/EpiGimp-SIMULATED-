//Application implementation
#include "../../include/Core/Application.hpp"
#include "../../include/UI/Toolbar.hpp"
#include "../../include/UI/Canvas.hpp"
#include "../../include/Utils/Implementations.hpp"
#include <iostream>
#include <stdexcept>

namespace EpiGimp {

Application::Application(AppConfig config) 
    : config_(std::move(config)), running_(false), initialized_(false)
    {
    
    eventDispatcher_ = std::make_unique<EventDispatcher>();
    
    std::cout << "Application created with config: " 
              << config_.windowWidth << "x" << config_.windowHeight 
              << " '" << config_.windowTitle << "'" << std::endl;
}

bool Application::initialize()
{
    if (initialized_) {
        std::cout << "Application already initialized" << std::endl;
        return true;
    }

    try {
        // Initialize window
        window_ = std::make_unique<WindowResource>(
            config_.windowWidth, config_.windowHeight, config_.windowTitle
        );
        
        if (!window_->isInitialized()) {
            throw std::runtime_error("Failed to initialize window");
        }

        errorHandler_ = std::make_unique<ConsoleErrorHandler>(eventDispatcher_.get());
        fileManager_ = std::make_unique<SimpleFileManager>();
        inputHandler_ = std::make_unique<RaylibInputHandler>();

        // Create UI components
        createComponents();
        setupEventHandlers();

        // Load initial image if specified
        if (!config_.initialImagePath.empty())
            canvas_->loadImage(config_.initialImagePath);

        initialized_ = true;
        std::cout << "Application initialized successfully" << std::endl;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize application: " << e.what() << std::endl;
        return false;
    }
}

void Application::run()
{
    if (!initialized_) {
        if (!initialize())
            return;
    }

    running_ = true;
    std::cout << "Application starting main loop" << std::endl;

    auto lastTime = GetTime();
    
    while (running_ && !window_->shouldClose()) {
        const auto currentTime = GetTime();
        const float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        update(deltaTime);
        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        draw();
        EndDrawing();
    }

    shutdown();
}

void Application::shutdown()
{
    if (!running_) return;
    
    running_ = false;
    std::cout << "Application shutting down..." << std::endl;
    
    // Components will be cleaned up automatically by unique_ptr destructors
    // Window will be closed automatically by WindowResource destructor
    
    std::cout << "Application shut down successfully" << std::endl;
}

void Application::update(float deltaTime)
{
    inputHandler_->update();
    handleEvents();
    
    // Handle file dialogs
    auto simpleFileManager = static_cast<SimpleFileManager*>(fileManager_.get());
    
    // Check for open dialog result
    auto openResult = simpleFileManager->updateOpenDialog();
    if (openResult) {
        canvas_->loadImage(*openResult);
    }
    
    // Check for save dialog result  
    auto saveResult = simpleFileManager->updateSaveDialog();
    if (saveResult) {
        canvas_->saveImage(*saveResult);
    }
    
    // Only update other components if no dialog is showing
    if (!simpleFileManager->isShowingDialog()) {
        if (toolbar_) toolbar_->update(deltaTime);
        if (canvas_) canvas_->update(deltaTime);
    }
}
    
void Application::draw()
{
    if (canvas_) canvas_->draw();
    if (toolbar_) toolbar_->draw();
    
    // Draw status bar
    const int statusY = config_.windowHeight - 25;
    DrawRectangle(0, statusY, config_.windowWidth, 25, LIGHTGRAY);
    DrawLine(0, statusY, config_.windowWidth, statusY, GRAY);
    
    const std::string statusText = canvas_->hasImage() 
        ? "Image loaded | Zoom: " + std::to_string(static_cast<int>(canvas_->getZoom() * 100)) + "%"
        : "Ready - Load an image to get started";
    
    DrawText(statusText.c_str(), 10, statusY + 5, 14, BLACK);
    
    // Draw file dialogs on top of everything
    auto simpleFileManager = static_cast<SimpleFileManager*>(fileManager_.get());
    simpleFileManager->updateOpenDialog();
    simpleFileManager->updateSaveDialog();
}

void Application::handleEvents()
{
    // Handle keyboard shortcuts
    if (inputHandler_->isKeyDown(KEY_LEFT_CONTROL)) {
        if (inputHandler_->isKeyPressed(KEY_O)) {
            onLoadImageRequest();
        } else if (inputHandler_->isKeyPressed(KEY_S)) {
            eventDispatcher_->emit<ImageSaveRequestEvent>("");
        }
    }
    
    if (inputHandler_->isKeyPressed(KEY_ESCAPE)) {
        running_ = false;
    }
}

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

} // namespace EpiGimp