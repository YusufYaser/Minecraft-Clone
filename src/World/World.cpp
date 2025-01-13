#include "World.h"
#include "Utils.h"
#include "../Game/Game.h"

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

	chunkLoader = std::thread([this]() {
		chunkLoaderFunc();
		});

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
}

World::~World() {
	unloading.store(true);
	print("Waiting for world tick thread...");
	tickThread.join();
	print("Waiting for chunk loader thread...");
	chunkLoader.join();
	print("Waiting for chunk unloader thread...");
	chunkUnloader.join();

	for (int c = 0; c < RENDERER_THREAD_COUNT; c++) {
		print("Waiting for rendering thread...");
		renderingThreads[c].join();
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
