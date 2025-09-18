//This is the header file for the toolbar UI components
#ifndef TOOLBARCLASS_HPP
#define TOOLBARCLASS_HPP

#include <string>
#include <functional>
#include "raylib.h"

class ToolbarClass {
private:
    static const int TOOLBAR_HEIGHT = 60;
    static const int BUTTON_WIDTH = 120;
    static const int BUTTON_HEIGHT = 40;
    static const int BUTTON_MARGIN = 10;
    
    struct Button {
        Rectangle rect;
        std::string text;
        Color color;
        Color hoverColor;
        Color pressedColor;
        bool isHovered;
        bool isPressed;
        std::function<void()> onClick;
        
        Button(Rectangle r, const std::string& t, std::function<void()> callback) 
            : rect(r), text(t), onClick(callback) {
            color = LIGHTGRAY;
            hoverColor = GRAY;
            pressedColor = DARKGRAY;
            isHovered = false;
            isPressed = false;
        }
    };
    
    static Button loadButton;
    static Button saveButton;
    static bool initialized;

public:
    static void initialize(int screenWidth, std::function<void()> loadCallback, std::function<void()> saveCallback);
    static void update();
    static void draw();
    static int getToolbarHeight() { return TOOLBAR_HEIGHT; }
    
private:
    static void updateButton(Button& button);
    static void drawButton(const Button& button);
};

#endif // TOOLBARCLASS_HPP