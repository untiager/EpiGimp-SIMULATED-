//Main.cpp

#include "../include/Basics.hpp"

int basic_tests(int ac, char **av)
{
    if (ac < 2) {
        create_new_file("png/new_image.png");
        return 0;
    }
    const char *filePath = av[1];
    if (!verify_file_existence(filePath)) {
        ErrorClass::displayError("File does not exist: " + std::string(filePath));
        return 84;
    }
    ImageClass::displayImage(filePath);
    return 0;
}

int main_loop(int ac, char **av)
{
    return 0;
}

int main(int ac, char **av)
{
    basic_tests(ac, av);
    main_loop(ac, av);
    return 0;
}