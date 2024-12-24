#include <glad/gl.h>
#include "GameWindow.h"
#include "../Logging.h"

GameWindow::GameWindow(glm::ivec2 isize, const char* title) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	m_size = isize;

	glfwWindow = glfwCreateWindow(isize.x, isize.y, title, NULL, NULL);
	if (!glfwWindow) {
		return;
	}

	glfwMakeContextCurrent(glfwWindow);

	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	const GLFWvidmode* vm = glfwGetVideoMode(monitors[0]);

	glfwSetWindowPos(glfwWindow, (vm->width - m_size.x) / 2, (vm->height - m_size.y) / 2);

	gladLoadGL(glfwGetProcAddress);

	glViewport(0, 0, m_size.x, m_size.y);
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

void GameWindow::setFullscreen(bool fullscreen) {
	m_fullscreen = fullscreen;

	int count;
	GLFWmonitor** monitors = glfwGetMonitors(&count);
	const GLFWvidmode* vm = glfwGetVideoMode(monitors[0]);

	glfwSetWindowMonitor(glfwWindow, fullscreen ? glfwGetPrimaryMonitor() : NULL, 0, 0, vm->width, vm->height, vm->refreshRate);

	if (!fullscreen) {
		glfwSetWindowPos(glfwWindow, (vm->width - m_size.x) / 2, (vm->height - m_size.y) / 2);
		glfwSetWindowSize(glfwWindow, m_size.x, m_size.y);
	}
}
