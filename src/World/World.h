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

	Block* getBlock(glm::vec3 pos);
	void setBlock(glm::vec3 pos, BLOCK_TYPE type, bool replace = true);

private:
	std::unordered_map<int, Block*> blocks;
};
