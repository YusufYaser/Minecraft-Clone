#pragma once
#include "glm/glm.hpp"
#include "../Block/Block.h"
#include "stdint.h"

#define STRUCTURES_COUNT 2

enum class STRUCTURE_TYPE : uint8_t {
	TREE = 0,
	HOUSE
};

struct StructureConfig {
	int priority = 0;
	float probability = 0.0f;
	glm::ivec3 size = { 0, 0, 0 };
	glm::ivec3 pivot = { 0, 0, 0 };
};

class Structure {
public:
	static void initialize();

	Structure(STRUCTURE_TYPE type, StructureConfig& settings, std::function<BLOCK_TYPE(glm::ivec2, glm::ivec3)> algorithm);

	STRUCTURE_TYPE getType() { return m_type; };
	float getProbability() { return m_probability; };
	int getPriority() { return m_priority; };
	BLOCK_TYPE getBlock(glm::ivec3 pos);

	bool isInXZ(glm::ivec2 pos); // please ignore that name
	int getBase(glm::ivec2 pos);
	int getHeight() { return m_size.y; };
	glm::ivec3 getPivot() { return m_pivot; };

	static Structure* getStructure(STRUCTURE_TYPE type) { return structures[(int)type]; };

private:
	static bool initialized;

	float m_probability = 0;
	int m_priority = 0;
	glm::ivec3 m_size;
	glm::ivec3 m_pivot;
	std::function<BLOCK_TYPE(glm::ivec2, glm::ivec3)> m_getBlock;
	STRUCTURE_TYPE m_type;

	static Structure* structures[STRUCTURES_COUNT];
};
