//Application initialization and lifecycle management
#include "../../include/Core/Application.hpp"
#include "../../include/UI/Toolbar.hpp"
#include "../../include/UI/Canvas.hpp"
#include "../../include/Utils/Implementations.hpp"
#include <iostream>
#include <stdexcept>

namespace EpiGimp {

Application::Application(AppConfig config) 
    : config_(std::move(config)), running_(false), initialized_(false), currentTool_(DrawingTool::None)
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
        
        if (!window_->isInitialized())
            throw std::runtime_error("Failed to initialize window");

        errorHandler_ = std::make_unique<ConsoleErrorHandler>(eventDispatcher_.get());
        fileManager_ = std::make_unique<SimpleFileManager>();
        inputHandler_ = std::make_unique<RaylibInputHandler>();
        historyManager_ = std::make_unique<HistoryManager>(50); // Support up to 50 undo levels

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

} // namespace EpiGimp