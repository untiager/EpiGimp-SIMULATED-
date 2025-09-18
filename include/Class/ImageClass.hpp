// This is the header file for image processing
#ifndef IMAGECLASS_HPP
#define IMAGECLASS_HPP

#include <string>
#include <iostream>
#include "raylib.h"
#include "ErrorClass.hpp"
#include "../Init.hpp"

class ImageClass {
public:
    static void displayImage(const std::string &imagePath) {
        // Maximum display size
        const int maxWidth = 1920;
        const int maxHeight = 1080;

        Image image = LoadImage(imagePath.c_str());
        if (image.data == nullptr) {
            std::cerr << "Failed to load image: " << imagePath << std::endl;
            return;
        }

        std::cout << "Original image size: " << image.width << "x" << image.height << std::endl;

        // Resize if too large
        if (image.width > maxWidth || image.height > maxHeight) {
            float widthRatio = (float)maxWidth / image.width;
            float heightRatio = (float)maxHeight / image.height;
            float scale = (widthRatio < heightRatio) ? widthRatio : heightRatio;
            int newWidth = (int)(image.width * scale);
            int newHeight = (int)(image.height * scale);
            std::cout << "Resizing image to: " << newWidth << "x" << newHeight << std::endl;
            ImageResize(&image, newWidth, newHeight);
        }

        // Now safe to create window and texture
        InitWindow(image.width, image.height, "Image Viewer");
        SetTargetFPS(60);

        std::cout << "Texture being created: " << imagePath << std::endl;
        Texture2D texture = LoadTextureFromImage(image);

        std::cout << "Texture created successfully." << std::endl;

        if (texture.id == 0) {
            std::cerr << "Failed to create texture from image: " << imagePath << std::endl;
            UnloadImage(image);
            return;
        }

        std::cout << "Image width: " << image.width << std::endl;
        std::cout << "Image height: " << image.height << std::endl;

        UnloadImage(image);

        std::cout << "texture width: " << texture.width << std::endl;
        std::cout << "texture height: " << texture.height << std::endl;

        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(texture, 0, 0, WHITE);
            EndDrawing();
        }

        UnloadTexture(texture);
        CloseWindow();   
    }
};

#endif // IMAGECLASS_HPP