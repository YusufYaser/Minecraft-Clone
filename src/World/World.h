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

class World {
public:
	World(siv::PerlinNoise::seed_type seed);
	~World();

	void Render(Shader* shader);

	Block* getBlock(glm::ivec3 pos);
	Block* setBlock(glm::ivec3 pos, BLOCK_TYPE type, bool replace = true);

	siv::PerlinNoise::seed_type getSeed() const { return seed; };

	void loadChunk(glm::ivec2 pos, bool permanentlyLoaded = false);

	int chunksLoaded();
	int chunkLoadQueueCount();

	int getHeight(glm::ivec2 pos);
	double random(glm::ivec2 pos);

private:
	std::atomic<bool> unloading;

	siv::PerlinNoise::seed_type seed;

	struct Chunk {
		bool loaded;
		bool permanentlyLoaded;
		std::mutex blocksMutex;
		std::mutex renderingGroupsMutex;
		time_t lastRendered;
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
};
