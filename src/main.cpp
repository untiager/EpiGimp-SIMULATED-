//Main.cpp

#include "../include/Basics.hpp"

//Main.cpp

#include "../include/Basics.hpp"

int basic_tests(int ac, char **av)
{
    if (ac < 2) {
        // If no arguments, start the paint interface
        std::cout << "Starting paint interface..." << std::endl;
        PaintInterfaceClass::initialize(1920, 1080);
        PaintInterfaceClass::run();
        return 0;
    }
    const char *filePath = av[1];
    if (!verify_file_existence(filePath)) {
        ErrorClass::displayError("File does not exist: " + std::string(filePath));
        return 84;
    }
    
    // Start paint interface and load the specified image
    std::cout << "Starting paint interface with image: " << filePath << std::endl;
    PaintInterfaceClass::initialize(1920, 1080);
    
    // Load the image into the canvas
    CanvasClass::loadImage(filePath);
    
    PaintInterfaceClass::run();
    return 0;
}

int main_loop(int ac, char **av)
{
    // This function is no longer needed as we use PaintInterfaceClass::run()
    // But keeping it for backward compatibility
    return 0;
}

int main(int ac, char **av)
{
    basic_tests(ac, av);
    return 0;
}