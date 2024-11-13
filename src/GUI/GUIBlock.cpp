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

void GUIBlock::Render(Shader* shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	shader->activate();
	glUniformMatrix4fv(shader->getUniformLoc("model"), 1, GL_FALSE, glm::value_ptr(model));
	glUniform2fv(shader->getUniformLoc("guiPos"), 1, glm::value_ptr(position));
	glUniform1f(shader->getUniformLoc("guiScale"), scale);
	_block->Render(shader);
}

void GUIBlock::setBlock(BLOCK_TYPE type)
{
	delete _block;
	_block = new Block(type, glm::vec3());
}
