﻿#pragma once
#include <iostream>
#include "Colors.h"
#include <cstring>
#include <glm/glm.hpp>

#ifdef _WIN32
#define FILENAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
inline char* BASENAME_NO_EXT(const char* filename) {
	const char* dot = strrchr(filename, '.');
	size_t len = dot ? (dot - filename) : strlen(filename);
	char* name = new char[len + 1];
	strncpy(name, filename, len);
	name[len] = '\0';
	return name;
}

#define print(...)\
(std::cout << BOLD BRIGHT_BLUE "Info "\
            BRIGHT_GREEN "(" << BASENAME_NO_EXT(FILENAME) << ")"\
            BLUE " -> " RESET, printArgs(__VA_ARGS__), std::cout << RESET);

#define error(...)\
(std::cout << BOLD BRIGHT_RED "Err  "\
            BRIGHT_GREEN "(" << BASENAME_NO_EXT(FILENAME) << ":" << __LINE__ << ")"\
            RED " -> " RESET, printArgs(__VA_ARGS__), std::cout << RESET);

#define warn(...)\
(std::cout << BOLD BRIGHT_YELLOW "Warn "\
            BRIGHT_GREEN "(" << BASENAME_NO_EXT(FILENAME) << ":" << __LINE__ << ")"\
            YELLOW " -> " RESET, printArgs(__VA_ARGS__), std::cout << RESET);

#ifdef GAME_DEBUG
#include <thread>
#define debug(...)\
(std::cout << BOLD BRIGHT_MAGENTA "Debug "\
            BRIGHT_GREEN "(" << BASENAME_NO_EXT(FILENAME) << ":" << __LINE__ << ")"\
            BRIGHT_BLUE " (" << std::this_thread::get_id() << ")"\
            MAGENTA " -> " RESET, printArgs(__VA_ARGS__), std::cout << RESET);
#else
#define debug(...)
#endif

template<typename T>
void printArgs(T&& arg) {
	std::cout << std::forward<T>(arg) << std::endl;
}

template<typename T, typename... Args>
void printArgs(T&& first, Args&&... args) {
	std::cout << std::forward<T>(first) << " ";
	printArgs(std::forward<Args>(args)...);
}

namespace glm {
	inline std::ostream& operator<<(std::ostream& os, const glm::ivec2& vec) {
		return os << "(" << vec.x << ", " << vec.y << ")";
	}
	inline std::ostream& operator<<(std::ostream& os, const glm::vec3& vec) {
		return os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	}
}
