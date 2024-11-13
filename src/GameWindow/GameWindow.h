#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class GameWindow
{
public:
	GameWindow(int width, int height, const char* title);
	~GameWindow();

	GLFWwindow* getGlfwWindow();

	glm::vec2 getSize();

private:
	GLFWwindow* glfwWindow;
};
