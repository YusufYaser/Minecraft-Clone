#include "Shaders.h"
#include "../Logging.h"

Shader::Shader(const char* vertexSource, const char* fragmentSource)
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, 0);
	glCompileShader(vertexShader);
	GLint shaderCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &shaderCompiled);
	if (shaderCompiled == false) {
		error("Failed to compile shader");
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
		error("Failed to compile shader");
		GLint logLength = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
		std::string errorLog;
		errorLog.resize(logLength);
		glGetShaderInfoLog(fragmentShader, logLength, NULL, &errorLog[0]);
		error(errorLog);
		return;
	}

	ID = glCreateProgram();
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);

	glLinkProgram(ID);
	glGetProgramiv(fragmentShader, GL_LINK_STATUS, &shaderCompiled);
	if (shaderCompiled == false) {
		error("Failed to link shader");
		GLint logLength = 0;
		glGetProgramiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
		std::string errorLog;
		errorLog.resize(logLength);
		glGetProgramInfoLog(fragmentShader, logLength, NULL, &errorLog[0]);
		error(errorLog);
		return;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

Shader::~Shader()
{
	glDeleteProgram(ID);
}
