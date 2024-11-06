#include <glad/gl.h>
#include "GameWindow.h"
#include "../Logging.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

GameWindow::GameWindow(int width, int height, const char* title)
{
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!window) {
		return;
	}
	glfwMakeContextCurrent(window);
	gladLoadGL(glfwGetProcAddress);

	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	glfwSetKeyCallback(window, key_callback);
}

GameWindow::~GameWindow()
{
	glfwDestroyWindow(window);
}

GLFWwindow* GameWindow::getWindow()
{
	return window;
}
