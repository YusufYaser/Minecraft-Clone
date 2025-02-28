#pragma once

#include <glm/glm.hpp>
#include "../Block/Block.h"
#include "../Shaders/Shaders.h"
#include <unordered_map>
#include <vector>
#include <PerlinNoise/PerlinNoise.hpp>
#include <thread>
#include <future>
#include <deque>
#include <random>
#include <time.h>
#include "Structures.h"
#include "stdint.h"
#include "../config.h"
#include "../Mux.h"
#include "../Entity/Entity.h"

enum class Generator : uint8_t {
	Default = 0,
	Flat,
	Void
};

struct WorldSaveData {
	int tick;
	unsigned int seed;
	Generator generator;
	uint8_t structuresCount;
	STRUCTURE_TYPE structures[STRUCTURES_COUNT];
	float playerPos[3];
	float playerOrientation[3];
	bool playerFlying;
};

struct ChunkSaveData {
	BLOCK_TYPE blocks[MAX_HEIGHT][16][16];
};

struct WorldSettings {
	siv::PerlinNoise::seed_type seed = 0u;
	Generator generator = Generator::Default;
	std::vector<STRUCTURE_TYPE> structures = { STRUCTURE_TYPE::TREE, STRUCTURE_TYPE::HOUSE };
	int initialTick = 0;
};

class World {
public:
	World(WorldSettings& settings);
	~World();

	WorldSaveData* createWorldSaveData();

	void render();
	void dontRender();

	Block* getBlock(glm::ivec3 pos);
	Block* setBlock(glm::ivec3 pos, BLOCK_TYPE type, bool replace = true);
	void fillBlocks(glm::ivec3 start, glm::ivec3 end, BLOCK_TYPE type);

	siv::PerlinNoise::seed_type getSeed() const { return seed; };

	void loadChunk(glm::ivec2 pos, bool permanentlyLoaded = false);

	size_t chunksLoaded();
	size_t chunkLoadQueueCount();
	int chunksRendered() const { return m_chunksRendered; };
	int instancesRendered() const { return m_instancesRendered; };
	int blocksRendered() const { return m_blocksRendered; };
	int totalInstances() const { return static_cast<int>(instances.size()); };
	bool isChunkLoaded(glm::ivec2 cPos);

	// Get the height at a position from the world generator
	int getHeight(glm::ivec2 pos);
	double random(glm::ivec2 pos, int seed = 0);
	int getSeaLevel() const { return generator == Generator::Default ? 36 : 0; };

	void setTick(int tick) { m_tick.store(tick); }
	int getTick() const { return m_tick.load(); }
	int getTime();

	float getAmbientLight();

	void spawnEntity(Entity* entity) { entities.push_back(entity); };

private:
	std::atomic<bool> unloading;

	std::atomic<int> m_tick;

	siv::PerlinNoise::seed_type seed;
	Generator generator;

	std::vector<Structure*> structures;

	struct Chunk {
		bool loaded : 1 = false;
		bool permanentlyLoaded : 1 = false;
		bool modified : 1 = false;
		bool reserved : 5;
		Mux blocksMutex;
		Mux renderingGroupsMutex;
		time_t lastRendered = 0;
		Block* blocks[16 * 16 * MAX_HEIGHT];
		std::vector<Block*> blocksToRender;
		glm::ivec2 pos;

		~Chunk() {
			blocksMutex.lock();
			for (auto& block : blocks) {
				delete block;
			}
			blocksMutex.unlock();
			renderingGroupsMutex.lock();
			blocksToRender.clear();
			renderingGroupsMutex.unlock();
		}
	};

	std::unordered_map<std::size_t, Chunk*> chunks;
	Mux chunksMutex;

	void setRenderingGroup(Block* block);

	void chunkLoaderFunc();
	std::thread chunkUnloader;
	void chunkUnloaderFunc();
	std::thread tickThread;
	void tick(float delta);
	std::deque<Chunk*> chunkLoadQueue;
	Mux chunkLoadQueueMutex;

	siv::PerlinNoise perlin;
	siv::PerlinNoise perlin2;

	int m_chunksRendered = 0;
	int m_instancesRendered = 0;
	int m_blocksRendered = 0;

	bool m_worldRenderModified = true;

	struct Instance {
		BLOCK_STRUCTURE_TYPE bStructType;
		BlockStructureData* bStructData;
		uint8_t hiddenFaces;
		GLuint VBO;
		uint16_t offsetsCount;
		glm::vec4 offsets[MAX_INSTANCE_OFFSETS];
		bool transparent;
	};

	std::thread renderingThreads[RENDERER_THREAD_COUNT];
	std::thread chunkLoaderThreads[CHUNK_LOADER_THREAD_COUNT];
	Mux renderingQueueMutex;
	std::deque<Chunk*> renderingQueue;
	std::atomic<bool> rendered[RENDERER_THREAD_COUNT];

	Block* targetBlock = nullptr;

	Mux instancesMutex;
	std::vector<Instance*> instances;

	Mux instancesToInitMutex;
	std::vector<Instance*> instancesToInit;

	std::atomic<bool> rendering;
	void renderer(int c);

	std::vector<Entity*> entities;
};
