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

inline constexpr glm::ivec3 faceDirections[6] = {
	{ 0, 0, 1 },
	{ 0, 0, -1 },
	{ 1, 0, 0 },
	{ -1, 0, 0 },
	{ 0, -1, 0 },
	{ 0, 1, 0 }
};

inline constexpr bool blocksTransparencies[BLOCK_TYPE_COUNT] = {
	0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0
};

inline constexpr bool blocksCollisions[BLOCK_TYPE_COUNT] = {
	1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1
};

const char* getTextureName(BLOCK_TYPE type);
int getAnimationFrameCount(BLOCK_TYPE type);

#ifdef GAME_DEBUG
enum class BLOCK_PLACEMENT_METHOD {
	UNKNOWN = 0,
	SET_BLOCK,
	FILL_BLOCKS
};
#endif

class Block {
public:
	Block(BLOCK_TYPE type, glm::ivec3 pos, uint8_t hiddenFaces = 0) : type(type), pos(pos), hiddenFaces(hiddenFaces) {};

	void Render(Shader* shader, uint8_t additionalHiddenFaces, bool bindTexture = true);
	const char* getName() const { return getTextureName(type); };
	bool hasCollision() const { return blocksCollisions[(uint8_t)type]; };
	bool hasTransparency() const { return blocksTransparencies[(uint8_t)type]; };

#ifdef GAME_DEBUG
	BLOCK_PLACEMENT_METHOD dPlacementMethod = BLOCK_PLACEMENT_METHOD::UNKNOWN;
#endif

public:
	const glm::ivec3 pos;
	const BLOCK_TYPE type;
	uint8_t hiddenFaces;
};
