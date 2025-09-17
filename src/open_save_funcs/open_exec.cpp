//This file is for opening and saving a file that was chosen at the time of execution

#include "../../include/Basics.hpp"
#include "../../include/Open_Save.hpp"

int verify_file_existence(const char *filename)
{
    FILE *file = std::fopen(filename, "r");
    try {
        if (file) {
            std::fclose(file);
            return 1; // File exists
        } else {
            return 0; // File does not exist
        }
    } catch (...) {
        if (file) {
            std::fclose(file);
        }
        throw; // Re-throw the exception
    }
    return 0;
}

int create_new_file(const char *filename)
{
    FILE *file = std::fopen(filename, "w");
    try {
        if (file) {
            std::fclose(file);
            return 1; // File created successfully
        } else {
            return 0; // Failed to create file
        }
    } catch (...) {
        if (file) {
            std::fclose(file);
        }
        throw; // Re-throw the exception
    }
    return 0;
}


int open_image(const char *filename)
{
    if (!verify_file_existence(filename)) {
        ErrorClass::displayError("File does not exist: " + std::string(filename));
        return 84;
    }
    ImageClass::displayImage(filename);
    return 0;
}