#pragma once

#include "../Block/Block.h"
#include <glad/gl.h>

class GUIBlock {
public:
	GUIBlock(BLOCK_TYPE type = BLOCK_TYPE::STONE);
	~GUIBlock();

	void Render(GLuint shader);
	void setBlock(BLOCK_TYPE type);

	glm::vec2 position;
	GLfloat scale;

private:
	Block* _block = nullptr;
};
