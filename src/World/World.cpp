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
}

World::~World() {
	unloading.store(true);
	print("Waiting for world tick thread...");
	tickThread.join();
	print("Waiting for chunk loader thread...");
	chunkLoader.join();
	print("Waiting for chunk unloader thread...");
	chunkUnloader.join();
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

void World::render() {
	m_chunksRendered = false;

	Player* player = Game::getInstance()->getPlayer();
	if (player == nullptr) return;

	Shader* shader = Game::getInstance()->getShader();

	shader->activate();;
	shader->setUniform("view", player->getView());
	shader->setUniform("projection", player->getProjection());

	Block* targetBlock = nullptr;
	player->getTargetBlock(&targetBlock);

	if (targetBlock != nullptr) targetBlock->highlighted = true;

	glm::vec3 pos = player->getCameraPos();
	int renderDistance = Game::getInstance()->getRenderDistance();
	shader->setUniform("highlighted", false);
	shader->setUniform("isLiquidTop", false);

	float cosHalfFOV = std::cos(glm::radians(190.0f / 2.0f));

	glm::ivec2 playerChunk = getPosChunk(player->pos);

	std::unordered_map<Chunk*, std::vector<BLOCK_TYPE>> queued;

	shader->setUniform("ambientLight", getAmbientLight());

	for (int x = -(renderDistance / 2) + static_cast<int>(round(pos.x / 16)); x < (renderDistance / 2) + static_cast<int>(round(pos.x / 16)); x++) {
		for (int y = -(renderDistance / 2) + static_cast<int>(round(pos.z / 16)); y < (renderDistance / 2) + static_cast<int>(round(pos.z / 16)); y++) {
			glm::ivec2 cPos = glm::ivec2(x, y);

			if (glm::length(glm::vec2(cPos - playerChunk)) > 1) {
				glm::vec3 chunkCenter = glm::vec3(x * 16 + 8, 0.0f, y * 16 + 8);
				glm::vec3 playerPos3D = pos;

				glm::vec3 toChunk = glm::normalize(chunkCenter - playerPos3D);
				glm::vec3 viewDir = glm::normalize(player->orientation);

				if (glm::dot(toChunk, viewDir) < cosHalfFOV) continue;
			}

			std::size_t chunkCh = hashPos(cPos);
			chunksMutex.lock();
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

			if (!chunk->renderingGroupsMutex.try_lock()) continue;

			m_chunksRendered++;

			for (auto& [type, blocks] : chunk->renderingGroups) {
				double startTime = glfwGetTime();
				if (type == BLOCK_TYPE::NONE || type == BLOCK_TYPE::AIR) continue;
				if (isBlockTypeTransparent(type)) { // TODO: do something better than this
					queued[chunk].push_back(type);
					continue;
				}
				glBindTexture(GL_TEXTURE_2D, getTexture(getTextureName(type))->id);

				for (auto& block : blocks) {
					if (block == nullptr) continue;
					block->Render(shader, false);
				}
			}
			chunk->lastRendered = time(nullptr);
			chunk->renderingGroupsMutex.unlock();
		}
	}

	for (auto& [chunk, types] : queued) {
		for (auto& type : types) {
			if (!chunk->renderingGroupsMutex.try_lock()) continue;
			auto& blocks = chunk->renderingGroups[type];
			chunk->renderingGroupsMutex.unlock();
			glBindTexture(GL_TEXTURE_2D, getTexture(getTextureName(type))->id);

			for (auto& block : blocks) {
				if (block == nullptr) continue;
				bool isLiquidTop = false;
				if (type == BLOCK_TYPE::WATER) {
					if (getBlock(block->getPos() + glm::ivec3(0, 1, 0)) == nullptr) {
						isLiquidTop = true;
					}
				}
				shader->setUniform("isLiquidTop", isLiquidTop);
				block->Render(shader, false);
			}
		}
	}

	if (targetBlock != nullptr) targetBlock->highlighted = false;
}

void World::tick() {
	m_tick++;
}

float World::getAmbientLight() {
	const double halfPi = acos(0.0); // 3.14 / 2

	float light = static_cast<float>((sin(((getTime() / 12000.0f) + 1) * halfPi) + 1.0f) / 2.0f);
	light *= 1 - .1f;
	return light + .1f;
}

int World::getTime() {
	return getTick();
}
