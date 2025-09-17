//This is the header file for opening and saving a file

#ifndef OPEN_SAVE_HPP
    #define OPEN_SAVE_HPP
    #include <cstdio>

int verify_file_existence(const char *filename);
int create_new_file(const char *filename);

#endif // OPEN_SAVE_HPP