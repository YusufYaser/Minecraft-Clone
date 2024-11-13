#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Textures/Textures.h"
#include "../Shaders/Shaders.h"
#include <glad/gl.h>
#include <unordered_map>

enum class BLOCK_TYPE : uint8_t {
	AIR = 0,
	STONE,
	GRASS,
	DIRT,
	OAK_LOG,
	OAK_LEAVES,
};

const int BLOCK_TYPE_COUNT = 6;

enum class BLOCK_FACE : uint8_t {
	FRONT = 0,
	BACK,
	RIGHT,
	LEFT,
	BOTTOM,
	TOP,
};

glm::ivec3 getBlockFaceDirection(BLOCK_FACE face);
const char* getTextureName(BLOCK_TYPE type);

class Block {
public:
	Block(BLOCK_TYPE type, glm::ivec3 pos, uint8_t hiddenFaces = 0);
	~Block();

	void Render(Shader* shader, bool bindTexture = true);
	const char* getName();
	glm::ivec3 getPos();
	BLOCK_TYPE getType();

	bool highlighted = false;
	uint8_t hiddenFaces;

private:
	BLOCK_TYPE type;
	glm::ivec3 pos;

	struct BlockStructureData {
		GLuint VAO, VBO, EBO;
		uint8_t faceCount;
	};

	static BlockStructureData blockStructures[64];
};
