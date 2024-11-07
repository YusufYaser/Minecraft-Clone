#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include "../Block/Block.h"
#include <unordered_map>
#include <PerlinNoise/PerlinNoise.hpp>

class World {
public:
	World(siv::PerlinNoise::seed_type seed, glm::vec2 size = glm::vec2(20, 20));
	~World();

	void Render(GLuint shader);

	Block* getBlock(glm::ivec3 pos);
	void setBlock(glm::ivec3 pos, BLOCK_TYPE type, bool replace = true);

	siv::PerlinNoise::seed_type getSeed();

private:
	std::unordered_map<int, Block*> blocks;
	siv::PerlinNoise::seed_type seed;
};
