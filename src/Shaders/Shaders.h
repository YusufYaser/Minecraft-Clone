#pragma once
#include <glad/gl.h>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
	Shader(const char* vertexSource, const char* fragmentSource, const char* name);
	~Shader();

	void activate();
	GLuint getUniformLoc(const GLchar* name);

	template<typename T>
	void setUniform(const GLchar* name, const T& value);

	bool successfullyLoaded() { return m_successfullyLoaded; };

private:
	GLuint m_id = 0;

	std::unordered_map<const GLchar*, GLuint> m_uniformLocs;

	bool m_successfullyLoaded = false;
};
