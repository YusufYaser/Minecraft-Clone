#pragma once
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../Textures/Textures.h"
#include "../Shaders/Shaders.h"
#include <glad/gl.h>

enum class BLOCK_TYPE : uint8_t {
	NONE = 0,
	AIR,
	STONE,
	GRASS,
	DIRT,
	OAK_LOG,
	OAK_LEAVES,
	WATER,
	BEDROCK,
	SAND,
	OAK_PLANKS,
	FLOWER,
	SANDSTONE
};

#define BLOCK_TYPE_COUNT 13

enum class BLOCK_FACE : uint8_t {
	FRONT = 0,
	BACK,
	RIGHT,
	LEFT,
	BOTTOM,
	TOP,
};

enum class BLOCK_STRUCTURE_TYPE : uint8_t {
	FULL_BLOCK = 0,
	PLANT
};

struct BlockStructureData {
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	uint8_t hiddenFaces;
	uint8_t faceCount;
};

BlockStructureData* getBlockStructureData(BLOCK_STRUCTURE_TYPE type, uint8_t hiddenFaces = 0);
BlockStructureData* createBlockStructureData(BLOCK_STRUCTURE_TYPE type, uint8_t hiddenFaces = 0);

BLOCK_STRUCTURE_TYPE getStructureType(BLOCK_TYPE type);

glm::ivec3 getBlockFaceDirection(BLOCK_FACE face);
const char* getTextureName(BLOCK_TYPE type);
int getAnimationFrameCount(BLOCK_TYPE type);
bool blockTypeHasCollision(BLOCK_TYPE type);
bool isBlockTypeTransparent(BLOCK_TYPE type);

#ifdef GAME_DEBUG
enum class BLOCK_PLACEMENT_METHOD {
	UNKNOWN = 0,
	SET_BLOCK,
	FILL_BLOCKS
};
#endif

class Block {
public:
	Block(BLOCK_TYPE type, glm::ivec3 pos, uint8_t hiddenFaces = 0);

	void Render(Shader* shader, uint8_t additionalHiddenFaces, bool bindTexture = true);
	const char* getName() const { return getTextureName(type); };
	glm::ivec3 getPos() const { return pos; };
	BLOCK_TYPE getType() const { return type; };
	bool hasCollision() const { return blockTypeHasCollision(type); };
	bool hasTransparency() const { return isBlockTypeTransparent(type); };

#ifdef GAME_DEBUG
	BLOCK_PLACEMENT_METHOD dPlacementMethod = BLOCK_PLACEMENT_METHOD::UNKNOWN;
#endif

private:
	glm::ivec3 pos;
	BLOCK_TYPE type;

public:
	uint8_t hiddenFaces;
};
