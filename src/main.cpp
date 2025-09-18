//Main.cpp - Modern C++ architecture

#include <iostream>
#include <exception>
#include <string>
#include "../include/Core/Application.hpp"

int main(int argc, char** argv) try {
    using namespace EpiGimp;
    
    AppConfig config;
    config.windowWidth = 1920;
    config.windowHeight = 1080;
    config.windowTitle = "EpiGimp - Paint Interface";
    config.targetFPS = 60;
    
    if (argc >= 2) {
        config.initialImagePath = argv[1];
        std::cout << "Starting with initial image: " << config.initialImagePath << std::endl;
    } else {
        std::cout << "Starting without initial image" << std::endl;
    }
    
    Application app(config);
    
    if (!app.initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return 1;
    }
    
    app.run();
    
    std::cout << "Application exited normally" << std::endl;
    return 0;
    
} catch (const std::exception& e) {
    std::cerr << "Unhandled exception: " << e.what() << std::endl;
    return 1;
} catch (...) {
    std::cerr << "Unknown exception occurred" << std::endl;
    return 1;
}