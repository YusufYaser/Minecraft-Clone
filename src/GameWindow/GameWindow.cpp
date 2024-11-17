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

	glfwWindow = glfwCreateWindow(width, height, title, NULL, NULL);
	if (!glfwWindow) {
		return;
	}
	glfwMakeContextCurrent(glfwWindow);

	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	const GLFWvidmode* videoMode = glfwGetVideoMode(monitors[0]);

	int mWidth = videoMode->width;
	int mHeight = videoMode->height;

	glfwSetWindowPos(glfwWindow, (mWidth - width) / 2, (mHeight - height) / 2);

	gladLoadGL(glfwGetProcAddress);

	glfwGetFramebufferSize(glfwWindow, &width, &height);
	glViewport(0, 0, width, height);

	glfwSetKeyCallback(glfwWindow, key_callback);
}

GameWindow::~GameWindow()
{
	glfwDestroyWindow(glfwWindow);
}

GLFWwindow* GameWindow::getGlfwWindow()
{
	return glfwWindow;
}

glm::vec2 GameWindow::getSize()
{
	int width, height;
	glfwGetWindowSize(getGlfwWindow(), &width, &height);
	return glm::vec2(width, height);
}
