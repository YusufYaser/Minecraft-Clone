#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include "../Block/Block.h"
#include <unordered_map>
#include <vector>
#include <PerlinNoise/PerlinNoise.hpp>
#include <thread>
#include <future>
#include <mutex>

class World {
public:
	World(siv::PerlinNoise::seed_type seed, float* progress, glm::ivec2 size = glm::ivec2(20, 20));
	~World();

	void Render(GLuint shader, glm::vec3 pos, float renderDistance = 50.0f);

	Block* getBlock(glm::ivec3 pos);
	void setBlock(glm::ivec3 pos, BLOCK_TYPE type, bool replace = true);

	siv::PerlinNoise::seed_type getSeed() const { return seed; };

private:
	std::unordered_map<std::size_t, Block*> blocks;
	std::mutex blocksMutex;
	std::unordered_map<BLOCK_TYPE, std::vector<Block*>> renderingGroups;
	siv::PerlinNoise::seed_type seed;

	bool loading = true;

	void setRenderingGroup(Block* block);
};
