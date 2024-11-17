#pragma once
#include "glm/glm.hpp"
#include "../Block/Block.h"
#include "stdint.h"

enum class STRUCTURE_TYPE : uint8_t {
	TREE = 0
};

#define STRUCTURES_COUNT 1

class Structure {
public:
	static void initialize();

	Structure(float probability, glm::ivec3 size, std::function<BLOCK_TYPE(glm::ivec2, glm::ivec3)> algorithm);

	float getProbability() { return m_probability; };
	BLOCK_TYPE getBlock(glm::ivec3 pos);

	int getBase(glm::ivec2 pos);
	int getHeight();

	static Structure* getStructure(STRUCTURE_TYPE type);

private:
	static bool initialized;

	float m_probability;
	glm::ivec3 m_size;
	std::function<BLOCK_TYPE (glm::ivec2, glm::ivec3)> m_getBlock;

	static Structure* structures[STRUCTURES_COUNT];
};
