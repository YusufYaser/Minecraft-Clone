#pragma once
#include <glad/gl.h>

class Shader {
public:
	Shader(const char* vertexSource, const char* fragmentSource);
	~Shader();

	GLuint ID;
};
