#include <glad/gl.h>
#include "GameWindow.h"
#include "../Logging.h"
#include <stb/stb_image.h>
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#undef APIENTRY
#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
#endif

GameWindow::GameWindow(glm::ivec2 isize, const char* title) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DEPTH_BITS, 32);
#ifdef GAME_DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_FLOATING, GL_TRUE);
#endif

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

	gladLoadGL();

	glViewport(0, 0, m_size.x, m_size.y);

	const char* img = "assets/textures/grass.png";

	switch (time(0) % 5) {
	case 2:
		img = "assets/textures/stone.png";
		break;
	case 3:
		img = "assets/textures/sand.png";
		break;
	case 4:
		img = "assets/textures/oak_log.png";
		break;
	case 5:
		img = "assets/textures/oak_planks.png";
		break;
	default:
		break;
	}

#ifdef _WIN32
	BOOL darkMode = TRUE;
	DwmSetWindowAttribute(glfwGetWin32Window(glfwWindow), 20, &darkMode, sizeof(BOOL));
#endif

	GLFWimage images[1];
	images[0].pixels = stbi_load(img, &images[0].width, &images[0].height, 0, 4);
	if (!images[0].pixels) {
		return;
	}
	images[0].height = images[0].width; // since textures are now 3D we get only the top face
	glfwSetWindowIcon(glfwWindow, 1, images);
	stbi_image_free(images[0].pixels);
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
