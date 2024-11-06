#pragma once
#include <GLFW/glfw3.h>

class GameWindow
{
public:
	GameWindow(int width, int height, const char* title);
	~GameWindow();

	GLFWwindow* getWindow();

private:
	GLFWwindow* window;
};
