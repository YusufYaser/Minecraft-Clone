#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>
#include "../Textures/Textures.h"

enum class BLOCK_TYPE {
	AIR = 0,
	STONE
};

class Block {
public:
	Block(BLOCK_TYPE type, glm::vec3 pos);
	~Block();

	void Render(GLuint shader);
	const char* getName();

private:
	GLuint VAO, VBO, EBO;
	BLOCK_TYPE type;
	glm::vec3 pos;
};
