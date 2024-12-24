#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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

private:
	GLFWwindow* glfwWindow = nullptr;
	glm::ivec2 m_size;

	bool m_fullscreen = false;
};
