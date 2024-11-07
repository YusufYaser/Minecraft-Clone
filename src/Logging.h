#pragma once
#include <iostream>

#define print(...)\
(std::cout << "Game | ", printArgs(__VA_ARGS__));

#define error(...)\
(std::cout << "Game | Error | ", printArgs(__VA_ARGS__));

template<typename T>
void printArgs(T&& arg) {
    std::cout << std::forward<T>(arg) << std::endl;
}

template<typename T, typename... Args>
void printArgs(T&& first, Args&&... args) {
    std::cout << std::forward<T>(first) << " ";
    printArgs(std::forward<Args>(args)...);
}
