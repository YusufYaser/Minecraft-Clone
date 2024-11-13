#pragma once
#include <iostream>
#include "Colors.h"

#ifdef _WIN32
#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
#define BASENAME_NO_EXT(filename) ([] { \
    static char name_without_ext[256];\
    const char* dot = strrchr(filename, '.');\
    size_t len = dot ? (dot - filename) : strlen(filename);\
    strncpy_s(name_without_ext, filename, len);\
    name_without_ext[len] = '\0';\
    return name_without_ext;\
})()

#define print(...)\
(std::cout << BOLD << BRIGHT_BLUE << "Info "\
            BRIGHT_GREEN << "(" << BASENAME_NO_EXT(FILENAME) << ")"\
            << BLUE << " -> " << RESET, printArgs(__VA_ARGS__), std::cout << RESET);

#define error(...)\
(std::cout << BOLD << BRIGHT_RED << "Err  "\
            BRIGHT_GREEN << "(" << BASENAME_NO_EXT(FILENAME) << ")"\
            << RED << " -> " << RESET, printArgs(__VA_ARGS__), std::cout << RESET);

template<typename T>
void printArgs(T&& arg) {
    std::cout << std::forward<T>(arg) << std::endl;
}

template<typename T, typename... Args>
void printArgs(T&& first, Args&&... args) {
    std::cout << std::forward<T>(first) << " ";
    printArgs(std::forward<Args>(args)...);
}
