#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Textures/Textures.h"
#include <glad/gl.h>

enum class BLOCK_TYPE {
	AIR = 0,
	STONE,
	GRASS,
	DIRT
};

class Block {
public:
	Block(BLOCK_TYPE type, glm::vec3 pos);
	~Block();

	void Render(GLuint shader);
	const char* getName();
	glm::vec3 getPos();

	bool highlighted = false;

private:
	GLuint VAO, VBO, EBO;
	BLOCK_TYPE type;
	glm::vec3 pos;
};
