#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#ifdef _WIN32
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#undef min
#undef max
#endif

class GameWindow {
public:
	GameWindow(glm::ivec2 isize, const char* title);
	~GameWindow();

	void update();

	GLFWwindow* getGlfwWindow() { return glfwWindow; };

	glm::vec2 getSize();
	bool isFocused();

	void setFullscreen(bool fullscreen);
	bool isFullscreen() const { return m_fullscreen; }

#ifdef _WIN32
	HWND getWin32Handle() { return glfwGetWin32Window(glfwWindow); };
#endif

private:
	GLFWwindow* glfwWindow = nullptr;
	glm::ivec2 m_size;

	bool m_fullscreen = false;
};
