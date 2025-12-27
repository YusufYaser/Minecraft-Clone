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
	Void,
	Debug
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
	std::string name = "World";
	siv::PerlinNoise::seed_type seed = 0u;
	Generator generator = Generator::Default;
	std::vector<STRUCTURE_TYPE> structures = { STRUCTURE_TYPE::TREE, STRUCTURE_TYPE::HOUSE };
	int initialTick = 0;
	bool internalWorld = false;
};

#ifdef GAME_DEBUG
enum class CHUNK_LOAD_METHOD {
	UNKNOWN = 0,
	GENERATED,
	FILE
};
#endif

struct Chunk {
	bool loaded : 1 = false;
	bool permanentlyLoaded : 1 = false;
	bool modified : 1 = false;
	bool reserved : 5;
	Mux blocksMutex;
	Mux renderingGroupsMutex;
	time_t lastRendered = 0;
	std::vector<Block*> blocks;
	int maxHeight = -1;
	std::vector<Block*> blocksToRender;
	glm::ivec2 pos;

#ifdef GAME_DEBUG
	CHUNK_LOAD_METHOD dLoadMethod = CHUNK_LOAD_METHOD::UNKNOWN;
#endif

	void setMaxHeight(int height) {
		if (height <= maxHeight) return;

		maxHeight = height;
		blocks.resize((height + 1) * 16 * 16, 0);
	}

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

struct BlockOffsetData {
	glm::vec3 pos;
	uint8_t blockType;
};

struct Instance {
	BLOCK_STRUCTURE_TYPE bStructType;
	BlockStructureData* bStructData;
	uint8_t hiddenFaces;
	GLuint VBO;
	std::vector<BlockOffsetData> offsets;
	bool transparent;
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

	std::string getName() const { return !m_internalWorld ? m_name : "[internal:" + m_name + "]"; };
	siv::PerlinNoise::seed_type getSeed() const { return seed; };
	Generator getGenerator() const { return generator; };
	bool isInternal() const { return m_internalWorld; };

	void loadChunk(glm::ivec2 pos, bool permanentlyLoaded = false);
	bool saveAllChunks();
	bool saveWorld();
	// saveWorld() and saveAllChunks() if not previously ran during the last auto save interval
	bool autoSave();

	size_t chunksLoaded();
	size_t chunkLoadQueueCount();
	int chunksRendered() const { return m_chunksRendered; };
	int instancesRendered() const { return m_instancesRendered; };
	int blocksRendered() const { return m_blocksRendered; };
	int totalInstances() const { return static_cast<int>(instances.size()); };
	bool isChunkLoaded(glm::ivec2 cPos);
	Chunk* getChunk(glm::ivec2 cPos);

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

	std::string m_name = "World";
	bool m_internalWorld = false;

	siv::PerlinNoise::seed_type seed;
	Generator generator;

	std::vector<Structure*> structures;

	std::unordered_map<std::size_t, Chunk*> chunks;
	Mux chunksMutex;

	void setRenderingGroup(Block* block);

	bool saveChunk(Chunk* chunk);

	double lastAutoSaved = 0;

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
