#include "World.h"
#include "Utils.h"
#include "../Game/Game.h"

World::World(WorldSettings& settings) {
	World::seed = settings.seed;
	World::perlin = siv::PerlinNoise{ seed };
	World::generator = settings.generator;

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
}

World::~World() {
	unloading.store(true);
	print("Waiting for chunk loader thread...");
	chunkLoader.join();
	print("Waiting for chunk unloader thread...");
	chunkUnloader.join();

	print("Removing chunks");
	for (auto& [ch, chunk] : chunks) {
		// TODO: use chunk unloader instead
		delete chunk;
	}
}

void World::render(Shader* shader) {
	m_chunksRendered = false;

	Player* player = Game::getInstance()->getPlayer();
	if (player == nullptr) return;

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

	const int CHUNKS_TO_LOAD_SIZE = 256;
	glm::ivec2 chunksToLoad[CHUNKS_TO_LOAD_SIZE];
	int chunksToLoadc = 0;

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
				if (chunksToLoadc >= CHUNKS_TO_LOAD_SIZE) continue;
				chunksToLoad[chunksToLoadc++] = cPos;
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
			auto& blocks = chunk->renderingGroups[type];
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

	if (chunksToLoadc < 2) { // sorting doesn't really matter in that case
		for (int i = 0; i < chunksToLoadc; i++) {
			loadChunk(chunksToLoad[i]);
		}
	} else {
		auto sorter = [&pos](glm::ivec2 a, glm::ivec2 b) {
			glm::ivec2 pos2 = { pos.x, pos.z };
			glm::ivec2 aDiff = a - pos2;
			int aDist = aDiff.x * aDiff.x + aDiff.y * aDiff.y;

			glm::ivec2 bDiff = b - pos2;
			int bDist = bDiff.x * bDiff.x + bDiff.y * bDiff.y;

			return aDist < bDist;
			};

		std::sort(chunksToLoad, chunksToLoad + chunksToLoadc, sorter);

		for (int i = 0; i < chunksToLoadc; i++) {
			loadChunk(chunksToLoad[i]);
		}
	}

	if (targetBlock != nullptr) targetBlock->highlighted = false;
}

float World::getAmbientLight() {
	float light = (sin((getTime() / 1200.0f) * 3.14f / 2.0f) + 1.0f) / 2.0f;
	light *= 1 - .1f;
	return light + .1f;
}

int World::getTime() {
	// For obvious reasons, this is not the best way to do it
	// I'll be keeping it like that for now until I add a tick system
	return glfwGetTime() * 10;
}
