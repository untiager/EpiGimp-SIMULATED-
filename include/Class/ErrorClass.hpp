// This is the header file for error handling

#ifndef ERRORCLASS_HPP
#define ERRORCLASS_HPP
#include <string>
#include <iostream>

class ErrorClass {
public:
    static void displayError(const std::string &message) {
        std::cerr << "Error: " << message << std::endl;
    }
};

#endif // ERRORCLASS_HPP