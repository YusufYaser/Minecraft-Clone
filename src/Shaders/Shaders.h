#pragma once
#include <glad/gl.h>

class Shader {
public:
	Shader(const char* vertexSource, const char* fragmentSource);
	~Shader();

	void activate();
	GLuint getUniformLoc(const GLchar* name);

	bool successfullyLoaded() { return m_successfullyLoaded; };

private:
	GLuint m_id;

	bool m_successfullyLoaded = false;
};
