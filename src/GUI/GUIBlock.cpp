#include "GUIBlock.h"

GUIBlock::GUIBlock(BLOCK_TYPE type)
{
	_block = new Block(type, glm::ivec3());
	position = glm::vec2();
	scale = 1.0f;
}

GUIBlock::~GUIBlock()
{
	delete _block;
}

void GUIBlock::Render(GLuint shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform2fv(glGetUniformLocation(shader, "guiPos"), 1, glm::value_ptr(position));
	glUniform1f(glGetUniformLocation(shader, "guiScale"), scale);
	_block->Render(shader);
}

void GUIBlock::setBlock(BLOCK_TYPE type)
{
	delete _block;
	_block = new Block(type, glm::vec3());
}
