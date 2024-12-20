#include <glad/gl.h>
#include "GameWindow.h"
#include "../Logging.h"

GameWindow::GameWindow(int width, int height, const char* title) {
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

	glViewport(0, 0, width, height);
}

GameWindow::~GameWindow() {
	glfwDestroyWindow(glfwWindow);
}

void GameWindow::update() {
	static glm::ivec2 oldSize = glm::ivec2();
	glm::ivec2 size = getSize();
	if (size.x != oldSize.y || size.y != oldSize.y) {
		glViewport(0, 0, size.x, size.y);
	}
	oldSize = size;
}

glm::vec2 GameWindow::getSize() {
	int width, height;
	glfwGetWindowSize(getGlfwWindow(), &width, &height);
	if (width == 0) width = 2;
	if (height == 0) height = 2;
	return glm::vec2(width + width % 2, height + height % 2);
}

bool GameWindow::isFocused() {
	return glfwGetWindowAttrib(glfwWindow, GLFW_FOCUSED) == GLFW_TRUE;
}
