#pragma once

#include <glad/gl.h>
#define GLT_IMPLEMENTATION
#include <gltext/gltext.h>

class DebugText {
public:
	static void initialize();
	static void cleanup();
	static void render();

private:
	static GLTtext* m_debugText;
};
