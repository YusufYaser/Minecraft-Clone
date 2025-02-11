#include "World.h"
#include "Utils.h"
#include "../Game/Game.h"
#include <glad/gl.h>

World::World(WorldSettings& settings) {
	World::seed = settings.seed;
	World::perlin = siv::PerlinNoise{ seed };
	World::generator = settings.generator;
	m_tick = settings.initialTick;

	int structureCount = 0;
	for (STRUCTURE_TYPE structureType : settings.structures) {
		Structure* structure = Structure::getStructure(structureType);
		structures.push_back(structure);
	}

	std::sort(structures.begin(), structures.end(), [](Structure* a, Structure* b) {
		return a->getPriority() > b->getPriority();
		});

	unloading.store(false);

	chunkUnloader = std::thread([this]() {
		chunkUnloaderFunc();
		});

	tickThread = std::thread([this]() {
		Game* game = Game::getInstance();

		double lastTick = glfwGetTime();
		double delta = 0;

		while (!unloading) {
			double currentTime = glfwGetTime();

			if (game->gamePaused()) {
				// maintain the tick delta until the game gets unpaused
				lastTick = currentTime - delta;
				continue;
			}

			delta = currentTime - lastTick;

			if (delta >= 1.0 / 20) {
				tick();
				lastTick = currentTime;
			}
		}
		});

	for (int c = 0; c < RENDERER_THREAD_COUNT; c++) {
		renderingThreads[c] = std::thread([this](int c) {
			renderer(c);
			}, c);
	}

	for (int c = 0; c < CHUNK_LOADER_THREAD_COUNT; c++) {
		chunkLoaderThreads[c] = std::thread([this]() {
			chunkLoaderFunc();
			});
	}
}

World::~World() {
	unloading.store(true);
	print("Waiting for world tick thread...");
	tickThread.join();
	for (int c = 0; c < CHUNK_LOADER_THREAD_COUNT; c++) {
		print("Waiting for chunk loader thread...");
		chunkLoaderThreads[c].join();
	}
	print("Waiting for chunk unloader thread...");
	chunkUnloader.join();

	for (int c = 0; c < RENDERER_THREAD_COUNT; c++) {
		print("Waiting for rendering thread...");
		renderingThreads[c].join();
	}

	for (auto& i : instances) {
		glDeleteBuffers(1, &i->VBO);
		i->VBO = 0;

		glDeleteVertexArrays(1, &i->bStructData->VAO);
		i->bStructData->VAO = 0;

		glDeleteBuffers(1, &i->bStructData->VBO);
		i->bStructData->VBO = 0;

		delete i->bStructData;
		i->bStructData = nullptr;

		delete i;
		i = nullptr;
	}
	instances.clear();
}

void World::dontRender() {
	m_chunksRendered = 0;
	m_instancesRendered = 0;

	glm::vec3 pos = Game::getInstance()->getPlayer()->getCameraPos();
	glm::ivec2 playerChunk = getPosChunk(pos);
	int renderDistance = Game::getInstance()->getRenderDistance();

	for (int x = -(renderDistance / 2) + playerChunk.x; x < (renderDistance / 2) + playerChunk.x; x++) {
		for (int y = -(renderDistance / 2) + playerChunk.y; y < (renderDistance / 2) + playerChunk.y; y++) {
			glm::ivec2 cPos = glm::ivec2(x, y);

			if (glm::length(glm::vec2(cPos - playerChunk)) * 2 > renderDistance) continue;

			std::size_t chunkCh = hashPos(cPos);
			if (!chunksMutex.try_lock()) continue;
			std::unordered_map<std::size_t, Chunk*>::iterator it = chunks.find(chunkCh);
			if (it == chunks.end()) {
				chunksMutex.unlock();
				loadChunk(cPos);
				continue;
			}
			Chunk* chunk = it->second;
			chunksMutex.unlock();
			if (chunk == nullptr) continue;
			if (!chunk->loaded) continue;
			chunk->lastRendered = time(nullptr);
		}
	}
}

WorldSaveData* World::createWorldSaveData() {
	WorldSaveData* data = new WorldSaveData();
	data->tick = m_tick;
	data->seed = seed;
	data->generator = generator;
	int i = 0;
	for (auto& structure : structures) {
		data->structures[i++] = structure->getType();
	}
	data->structuresCount = i;

	glm::vec3 pos = Game::getInstance()->getPlayer()->pos;
	data->playerPos[0] = pos.x;
	data->playerPos[1] = pos.y;
	data->playerPos[2] = pos.z;

	return data;
}

void World::tick() {
	m_tick++;
}

float World::getAmbientLight() {
	const double halfPi = acos(0.0); // 3.14 / 2

	float light = static_cast<float>((sin(((getTime() / 6000.0f) + 1) * halfPi) + 1.0f) / 2.0f);
	light *= 1 - .1f;
	return light + .1f;
}

int World::getTime() {
	return getTick();
}
