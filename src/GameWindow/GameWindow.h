#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class GameWindow
{
public:
	GameWindow(int width, int height, const char* title);
	~GameWindow();

	void update();

	GLFWwindow* getGlfwWindow() { return glfwWindow; };

	glm::vec2 getSize();

private:
	GLFWwindow* glfwWindow;
};
