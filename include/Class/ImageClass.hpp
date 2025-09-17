// This is the header file for image processing
#ifndef IMAGECLASS_HPP
#define IMAGECLASS_HPP

#include <string>
#include <iostream>

class ImageClass {
public:
    static void displayImage(const std::string &imagePath) {
        std::cout << "Displaying image: " << imagePath << std::endl;
    }
};

#endif // IMAGECLASS_HPP