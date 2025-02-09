#include "Shaders.h"
#include "../Logging.h"
#include <fstream>
#include <sstream>

Shader::Shader(const char* vertexSource, const char* fragmentSource, const char* name) {
	print("Loading", name, "shader");

	std::string vertex;
	std::string fragment;

	{
		std::ifstream file("assets/shaders/" + std::string(name) + "/" + std::string(name) + ".vert", std::ios::binary);
		if (file.is_open()) {
			print("Using custom vertex shader in assets");
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			vertex = content;
			file.close();
		} else {
			vertex = vertexSource;
		}
	}

	{
		std::ifstream file("assets/shaders/" + std::string(name) + "/" + std::string(name) + ".frag", std::ios::binary);
		if (file.is_open()) {
			print("Using custom fragment shader in assets");
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			fragment = content;
			file.close();
		} else {
			fragment = fragmentSource;
		}
	}

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	const char* vertex2 = vertex.c_str();
	glShaderSource(vertexShader, 1, &vertex2, 0);
	glCompileShader(vertexShader);
	GLint shaderCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled == false) {
		error("Failed to compile vertex shader");
		GLint logLength = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
		std::string errorLog;
		errorLog.resize(logLength);
		glGetShaderInfoLog(vertexShader, logLength, NULL, &errorLog[0]);
		error(errorLog);
		return;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fragment2 = fragment.c_str();
	glShaderSource(fragmentShader, 1, &fragment2, 0);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled == false) {
		error("Failed to compile fragment shader");
		GLint logLength = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
		std::string errorLog;
		errorLog.resize(logLength);
		glGetShaderInfoLog(fragmentShader, logLength, NULL, &errorLog[0]);
		error(errorLog);
		return;
	}

	m_id = glCreateProgram();
	glAttachShader(m_id, vertexShader);
	glAttachShader(m_id, fragmentShader);

	glLinkProgram(m_id);
	glGetProgramiv(m_id, GL_LINK_STATUS, &shaderCompiled);
	if (shaderCompiled == false) {
		error("Failed to link shader");
		GLint logLength = 0;
		glGetProgramiv(m_id, GL_INFO_LOG_LENGTH, &logLength);
		std::string errorLog;
		errorLog.resize(logLength);
		glGetProgramInfoLog(m_id, logLength, NULL, &errorLog[0]);
		error(errorLog);
		return;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	m_successfullyLoaded = true;

	print("Loaded", name, "shader");
}

Shader::~Shader() {
	glDeleteProgram(m_id);
}

void Shader::activate() {
	glUseProgram(m_id);
}

GLuint Shader::getUniformLoc(const GLchar* name) {
	auto it = m_uniformLocs.find(name);
	if (it != m_uniformLocs.end()) {
		return it->second;
	}

	GLuint loc = glGetUniformLocation(m_id, name);
	if (loc == -1) return -1;

	m_uniformLocs[name] = loc;

	return loc;
}

template<>
void Shader::setUniform<GLuint>(const GLchar* name, const GLuint& val) {
	glUniform1i(getUniformLoc(name), val);
}

template<>
void Shader::setUniform<GLint>(const GLchar* name, const int& val) {
	glUniform1i(getUniformLoc(name), val);
}

template<>
void Shader::setUniform<GLdouble>(const GLchar* name, const GLdouble& val) {
	glUniform1d(getUniformLoc(name), val);
}

template<>
void Shader::setUniform<GLfloat>(const GLchar* name, const GLfloat& val) {
	glUniform1f(getUniformLoc(name), val);
}

template<>
void Shader::setUniform<bool>(const GLchar* name, const bool& val) {
	setUniform(name, val ? 1 : 0);
}

template<>
void Shader::setUniform<glm::vec2>(const GLchar* name, const glm::vec2& val) {
	glUniform2fv(getUniformLoc(name), 1, glm::value_ptr(val));
}

template<>
void Shader::setUniform<glm::ivec2>(const GLchar* name, const glm::ivec2& val) {
	glUniform2iv(getUniformLoc(name), 1, glm::value_ptr(val));
}

template<>
void Shader::setUniform<glm::vec3>(const GLchar* name, const glm::vec3& val) {
	glUniform3fv(getUniformLoc(name), 1, glm::value_ptr(val));
}

template<>
void Shader::setUniform<glm::vec4>(const GLchar* name, const glm::vec4& val) {
	glUniform4fv(getUniformLoc(name), 1, glm::value_ptr(val));
}

template<>
void Shader::setUniform<glm::ivec3>(const GLchar* name, const glm::ivec3& val) {
	glUniform3iv(getUniformLoc(name), 1, glm::value_ptr(val));
}

template<>
void Shader::setUniform<glm::mat4>(const GLchar* name, const glm::mat4& val) {
	glUniformMatrix4fv(getUniformLoc(name), 1, GL_FALSE, glm::value_ptr(val));
}
