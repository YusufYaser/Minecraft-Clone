#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Textures/Textures.h"
#include <glad/gl.h>

enum class BLOCK_TYPE : uint8_t {
	AIR = 0,
	STONE,
	GRASS,
	DIRT,
};

enum class BLOCK_FACE {
	FRONT = 0,
	BACK,
	RIGHT,
	LEFT,
	BOTTOM,
	TOP,
};

glm::ivec3 getBlockFaceDirection(BLOCK_FACE face);

class Block {
public:
	Block(BLOCK_TYPE type, glm::ivec3 pos, uint8_t hiddenFaces = 0);
	~Block();

	void Render(GLuint shader);
	const char* getName();
	glm::ivec3 getPos();

	void updateVertices();

	bool highlighted = false;
	uint8_t hiddenFaces;
	uint8_t faceCount;

private:
	GLuint VAO, VBO, EBO;
	BLOCK_TYPE type;
	glm::ivec3 pos;
};
