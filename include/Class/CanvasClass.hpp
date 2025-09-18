//This is the header file for the canvas where image editing will happen
#ifndef CANVASCLASS_HPP
#define CANVASCLASS_HPP

#include <string>
#include "raylib.h"

class CanvasClass {
private:
    static Texture2D currentTexture;
    static bool hasImage;
    static Rectangle canvasArea;
    static float zoomLevel;
    static Vector2 panOffset;
    
public:
    static void initialize(int x, int y, int width, int height);
    static void loadImage(const std::string& imagePath);
    static void saveImage(const std::string& imagePath);
    static void update();
    static void draw();
    static void cleanup();
    
    // Future painting methods (stubbed for now)
    static void drawPixel(int x, int y, Color color);
    static void drawLine(Vector2 start, Vector2 end, Color color, float thickness);
    static void drawCircle(Vector2 center, float radius, Color color);
    static void drawRectangle(Rectangle rect, Color color);
    
    // Utility methods
    static bool hasLoadedImage() { return hasImage; }
    static Rectangle getCanvasArea() { return canvasArea; }
    static Texture2D getCurrentTexture() { return currentTexture; }
};

#endif // CANVASCLASS_HPP