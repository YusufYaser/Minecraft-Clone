#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include "../Block/Block.h"
#include "../Shaders/Shaders.h"
#include <unordered_map>
#include <vector>
#include <PerlinNoise/PerlinNoise.hpp>
#include <thread>
#include <future>
#include <mutex>
#include <random>
#include <time.h>
#include "Structures.h"
#include "stdint.h"

enum class Generator : uint8_t {
	Default = 0,
	Flat,
	Void
};

struct WorldSettings {
	siv::PerlinNoise::seed_type seed = 0u;
	Generator generator = Generator::Default;
	std::vector<STRUCTURE_TYPE> structures = { STRUCTURE_TYPE::TREE, STRUCTURE_TYPE::HOUSE };
};

class World {
public:
	World(WorldSettings& settings);
	~World();

	void render(Shader* shader);

	Block* getBlock(glm::ivec3 pos);
	Block* setBlock(glm::ivec3 pos, BLOCK_TYPE type, bool replace = true);

	siv::PerlinNoise::seed_type getSeed() const { return seed; };

	void loadChunk(glm::ivec2 pos, bool permanentlyLoaded = false);

	size_t chunksLoaded();
	size_t chunkLoadQueueCount();
	int chunksRendered() { return m_chunksRendered; };

	// Get the height at a position from the world generator
	int getHeight(glm::ivec2 pos);
	double random(glm::ivec2 pos, int seed = 0);

private:
	std::atomic<bool> unloading;

	siv::PerlinNoise::seed_type seed;
	Generator generator;

	std::vector<Structure*> structures;

	struct Chunk {
		bool loaded = false;
		bool permanentlyLoaded = false;
		std::mutex blocksMutex;
		std::mutex renderingGroupsMutex;
		time_t lastRendered = 0;
		std::unordered_map<std::size_t, Block*> blocks;
		std::unordered_map<BLOCK_TYPE, std::vector<Block*>> renderingGroups;

		~Chunk() {
			blocksMutex.lock();
			for (auto& [ch, block] : blocks) {
				delete block;
			}
			blocks.clear();
			blocksMutex.unlock();
			renderingGroupsMutex.lock();
			for (auto& [ch, blocks] : renderingGroups) {
				blocks.clear();
			}
			renderingGroups.clear();
			renderingGroupsMutex.unlock();
		}
	};

	std::unordered_map<std::size_t, Chunk*> chunks;
	std::mutex chunksMutex;

	void setRenderingGroup(Block* block);

	std::thread chunkLoader;
	void chunkLoaderFunc();
	std::thread chunkUnloader;
	void chunkUnloaderFunc();
	std::vector<glm::ivec2> chunkLoadQueue;
	std::mutex chunkLoadQueueMutex;

	siv::PerlinNoise perlin;

	int m_chunksRendered = 0;
};
