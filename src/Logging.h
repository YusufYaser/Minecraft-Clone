#pragma once
#include <iostream>
#include "Colors.h"

#define print(...)\
(std::cout << BOLD << BRIGHT_BLUE << "Info "\
            BRIGHT_GREEN << "(game)" << BLUE << " -> " << RESET, printArgs(__VA_ARGS__), std::cout << RESET);

#define error(...)\
(std::cout << BOLD << BRIGHT_RED << "Err  "\
            BRIGHT_GREEN << "(game)" << RED << " -> " << RESET, printArgs(__VA_ARGS__), std::cout << RESET);

template<typename T>
void printArgs(T&& arg) {
    std::cout << std::forward<T>(arg) << std::endl;
}

template<typename T, typename... Args>
void printArgs(T&& first, Args&&... args) {
    std::cout << std::forward<T>(first) << " ";
    printArgs(std::forward<Args>(args)...);
}
