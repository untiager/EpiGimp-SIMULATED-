//This is the header file for the main paint interface
#ifndef PAINTINTERFACECLASS_HPP
#define PAINTINTERFACECLASS_HPP

#include <string>
#include "raylib.h"
#include "ToolbarClass.hpp"
#include "CanvasClass.hpp"

class PaintInterfaceClass {
private:
    static int screenWidth;
    static int screenHeight;
    static bool initialized;
    static std::string currentImagePath;

public:
    static void initialize(int width, int height);
    static void run();
    static void update();
    static void draw();
    static void cleanup();
    
private:
    static void onLoadButtonClick();
    static void onSaveButtonClick();
    static void handleKeyboardShortcuts();
    
    // File dialog functionality (simplified for now)
    static std::string showOpenFileDialog();
    static std::string showSaveFileDialog();
};

#endif // PAINTINTERFACECLASS_HPP