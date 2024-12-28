#include "Shaders.h"
#include "../Logging.h"

Shader::Shader(const char* vertexSource, const char* fragmentSource) {
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, 0);
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
	glShaderSource(fragmentShader, 1, &fragmentSource, 0);
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
