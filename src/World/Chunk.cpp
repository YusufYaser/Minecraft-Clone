#include "World.h"
#include "Utils.h"
#include "../Game/Game.h"

void World::chunkLoaderFunc() {
	while (!unloading.load()) {
		chunkLoadQueueMutex.lock();
		Chunk* chunk = nullptr;
		generatingChunks.store(!chunkLoadQueue.empty());
		if (generatingChunks.load()) {
			chunk = chunkLoadQueue.front();
			chunkLoadQueue.pop_front();
		}

		if (chunk == nullptr || chunk->loaded) {
			chunkLoadQueueMutex.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
			continue;
		}

		Player* player = Game::getInstance()->getPlayer();

		glm::ivec2 pos = chunk->pos;
		std::size_t chunkCh = hashPos(pos);
		chunkLoadQueueMutex.unlock();

		if (time(0) - chunk->lastRendered > 10 &&
			(player == nullptr || glm::length(glm::vec2(chunk->pos - getPosChunk(player->getPos()))) > Game::getInstance()->getRenderDistance() + PRELOAD_DISTANCE)
			) {
			delete chunk;
			chunkLoadQueueMutex.lock();
			chunks.erase(chunkCh);
			chunkLoadQueueMutex.unlock();
			continue;
		}

		if (!m_internalWorld) {
			std::string path = "worlds/" + m_name + "/c" + std::to_string(chunkCh);
			if (std::filesystem::exists(path)) {
				std::ifstream chunkFile(path, std::ios::binary);
				ChunkSaveData* saveData = new ChunkSaveData();
				chunkFile.read(reinterpret_cast<char*>(saveData), sizeof(ChunkSaveData));
				chunkFile.close();

				debug("Loading chunk", chunk->pos, "from file:", path, chunk);

				for (int y = 0; y < MAX_HEIGHT; y++) {
					for (int x = 0; x < 16; x++) {
						for (int z = 0; z < 16; z++) {
							if (saveData->blocks[y][x][z] == BLOCK_TYPE::AIR) continue;

							glm::ivec3 bPos = {
								x + (pos.x * 16),
								y,
								z + (pos.y * 16),
							};

							setBlock(bPos, saveData->blocks[y][x][z], false);
						}
					}
				}

				delete saveData;

				chunk->loaded = true;
				chunk->modified = false;

#ifdef GAME_DEBUG
				chunk->dLoadMethod = CHUNK_LOAD_METHOD::FILE;
#endif
				continue;
			}
		}

		if (generator == Generator::Debug) {
			for (int x = pos.x * 16; x < 16 + pos.x * 16; x++) {
				for (int z = pos.y * 16; z < 16 + pos.y * 16; z++) {
					setBlock(glm::ivec3(x, 0, z), std::abs(x) % 2 != std::abs(z) % 2 ? BLOCK_TYPE::STONE : BLOCK_TYPE::DIRT, false);

					if (z < 0) continue;

					if (x > -10 && x <= 0) {
						if (x % 2 != 0 || z % 2 != 0) continue;
						if ((z * 5 - x) / 2 >= BLOCK_TYPE_COUNT) continue;

						setBlock(glm::ivec3(x, 2, z), BLOCK_TYPE((z * 5 - x) / 2), false);
					}
				}
			}

			chunk->loaded = true;
			chunk->modified = false;
			continue;
		}

		if (generator == Generator::Void) {
			if (pos.x == 0 && pos.y == 0) {
				setBlock({ 0, 0, 0 }, BLOCK_TYPE::BEDROCK);
			}
			chunk->loaded = true;
			chunk->modified = false;
#ifdef GAME_DEBUG
			chunk->dLoadMethod = CHUNK_LOAD_METHOD::GENERATED;
#endif
			continue;
		}

		int seaLevel = getSeaLevel();

		int heightMap[16][16] = {};
		int minHeight = MAX_HEIGHT;

		for (int x = 0; x < 16; x++) {
			for (int z = 0; z < 16; z++) {
				heightMap[x][z] = getHeight({ x + pos.x * 16, z + pos.y * 16 });
				if (heightMap[x][z] < minHeight) minHeight = heightMap[x][z];
			}
		}

		minHeight -= 3;

		fillBlocks({ pos.x * 16, 0, pos.y * 16 }, { 15 + pos.x * 16, 0, 15 + pos.y * 16 }, BLOCK_TYPE::BEDROCK);
		if (minHeight >= 1) fillBlocks({ pos.x * 16, 1, pos.y * 16 }, { 15 + pos.x * 16, minHeight, 15 + pos.y * 16 }, BLOCK_TYPE::STONE);

		for (int x = pos.x * 16; x < 16 + pos.x * 16; x++) {
			for (int z = pos.y * 16; z < 16 + pos.y * 16; z++) {
				int height = heightMap[x - pos.x * 16][z - pos.y * 16];

				Biome biome = getBiome({ x, z });

				for (int y = minHeight; y < height; y++) {
					if (unloading.load()) break;
					BLOCK_TYPE type = BLOCK_TYPE::STONE;
					if (y == 0) {
						type = BLOCK_TYPE::BEDROCK;
					} else {
						switch (biome.type) {
						case BiomeType::Desert:
							if (fmin(random({ x, z }), 1.0f) <= fmin(biome.value / 0.02f, 1.0f)) {
								type = BLOCK_TYPE::SAND;
								break;
							}
						default:
							if (height > seaLevel) {
								if (y == height - 1) type = BLOCK_TYPE::GRASS;
								else if (y >= height - 3) {
									type = BLOCK_TYPE::DIRT;
								}
							} else {
								if (y == height - 1 || y == height - 2) type = BLOCK_TYPE::SAND;
								else type = BLOCK_TYPE::STONE;
							}
						}
					}

					setBlock(glm::ivec3(x, y, z), type, false);
				}
				if (height <= seaLevel) {
					for (int y = height; y < seaLevel; y++) {
						setBlock(glm::ivec3(x, y, z), BLOCK_TYPE::WATER, false);
					}
				}
				/*else {
					const double random = perlin2.octave2D_01((x * 0.025), (z * 0.025), 4);
					if (random > 0.95) {
						setBlock(glm::ivec3(x, height, z), BLOCK_TYPE::FLOWER, false);
					}
				}*/

				int MAX_STRUCTURE_HEIGHT = 0;

				std::vector<Structure*> structuresInChunk;

				bool genStructures = false;

				for (Structure* structure : structures) {
					if (!structure->isInXZ({ x, z })) continue;

					int sHeight = structure->getHeight();
					sHeight -= structure->getPivot().y;
					sHeight += height;
					if (sHeight > MAX_STRUCTURE_HEIGHT) MAX_STRUCTURE_HEIGHT = sHeight;

					structuresInChunk.push_back(structure);
					genStructures = true;
				}

				if (!genStructures) continue;

				for (int y = 0; y < height + MAX_STRUCTURE_HEIGHT; y++) {
					for (Structure* structure : structuresInChunk) {
						BLOCK_TYPE blockType = structure->getBlock({ x, y, z });
						if (blockType != BLOCK_TYPE::NONE) {
							setBlock(glm::ivec3(x, y, z), blockType, true);
							break;
						}
					}
				}
			}
		}

		chunk->loaded = true;
		chunk->modified = false;
#ifdef GAME_DEBUG
		chunk->dLoadMethod = CHUNK_LOAD_METHOD::GENERATED;
#endif
	}
}

void World::chunkUnloaderFunc() {
	while (true) {
		if (unloading.load() && chunks.size() == 0) break;
		if (!unloading.load()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if (Game::getInstance()->loadingWorld() && !unloading.load()) continue;
		time_t current = time(nullptr);
		chunksMutex.lock();

		for (auto& [ch, chunk] : chunks) {
			if (chunk == nullptr) {
				chunks.erase(ch);
				continue;
			}
			Player* player = Game::getInstance()->getPlayer();

			if (!unloading.load() && player != nullptr &&
				glm::length(glm::vec2(chunk->pos - getPosChunk(player->getPos()))) <= Game::getInstance()->getRenderDistance() + PRELOAD_DISTANCE) continue;

			if ((chunk->loaded && !chunk->permanentlyLoaded && current - chunk->lastRendered > 20)
				|| unloading.load()) {

				if (chunk->modified && chunk->loaded && !m_internalWorld) {
					saveChunk(chunk);
				}

				delete chunks[ch];
				if (!unloading.load()) {
					chunks.erase(ch);
					break;
				}
			}
		}
		if (unloading.load()) {
			chunks.clear();
		}
		chunksMutex.unlock();
	}
}

bool World::saveAllChunks() {
	if (m_internalWorld) return false;

	debug("Saving world");

	bool status = true;
	int savedCount = 0;

	chunksMutex.lock();

	for (auto& [ch, chunk] : chunks) {
		if (chunk == nullptr) continue;

		if (chunk->modified && chunk->loaded) {
			status = status && saveChunk(chunk);
			if (status) savedCount++;
		}
	}

	chunksMutex.unlock();

	if (status) {
		if (savedCount != 0) {
			print("Saved", savedCount, "chunks")
		} else {
			debug("Saved", savedCount, "chunks")
		}
	} else {
		error("Failed to save some chunks. Chunks saved:", savedCount);
	}

	return status;
}

bool World::autoSave() {
	double currentTime = glfwGetTime();
	if (currentTime - lastAutoSaved < AUTOSAVE_INTERVAL) return false;

	lastAutoSaved = currentTime;

	bool saveChunksStatus = saveAllChunks();
	bool saveWorldStatus = saveWorld();

	return saveChunksStatus && saveWorldStatus;
}

bool World::saveChunk(Chunk* chunk) {
	if (m_internalWorld) return false;

	debug("Saving chunk", chunk->pos);

	try {
		ChunkSaveData* saveData = new ChunkSaveData();
		chunk->blocksMutex.lock();
		for (auto& block : chunk->blocks) {
			if (block == nullptr) continue;
			glm::ivec3 rPos = block->getPos();
			glm::ivec2 cPos = getPosChunk(rPos);
			glm::ivec3 pos = {
				rPos.x - (cPos.x * 16),
				rPos.y,
				rPos.z - (cPos.y * 16),
			};
			saveData->blocks[pos.y][pos.x][pos.z] = block->getType();
		}
		chunk->blocksMutex.unlock();

		std::string path = "worlds/" + m_name + "/c" + std::to_string(hashPos(chunk->pos));
		std::ofstream chunkFile(path, std::ios::binary);
		chunkFile.write(reinterpret_cast<const char*>(saveData), sizeof(ChunkSaveData));
		chunk->modified = false;
		chunkFile.close();

		delete saveData;

		debug("Saved chunk", chunk->pos, "at", path);
		return true;
	} catch (std::filesystem::filesystem_error e) {
		error("FAILED TO SAVE CHUNK:", e.what());
	}

	return false;
}

void World::loadChunk(glm::ivec2 pos, bool permanentlyLoaded) {
	std::size_t chunkCh = hashPos(pos);
	chunksMutex.lock();
	auto it = chunks.find(chunkCh);

	if (it != chunks.end()) {
		chunksMutex.unlock();
		it->second->lastRendered = time(nullptr);
		return; // already loaded
	}
	chunksMutex.unlock();

	chunkLoadQueueMutex.lock();
	for (auto& c : chunkLoadQueue) {
		if (c->pos == pos) {
			chunkLoadQueueMutex.unlock();
			return;
		}
	}

	Chunk* chunk = new Chunk();
	chunk->permanentlyLoaded = permanentlyLoaded;
	chunk->lastRendered = time(nullptr);
	chunk->pos = pos;
	chunksMutex.lock();
	chunks[chunkCh] = chunk;
	chunksMutex.unlock();

	chunkLoadQueue.push_back(chunk);

	Player* player = Game::getInstance()->getPlayer();
	if (player != nullptr) {
		glm::ivec2 pPos = getPosChunk(player->getPos());

		std::sort(chunkLoadQueue.begin(), chunkLoadQueue.end(), [&pPos](Chunk* a, Chunk* b) {
			auto distanceSquared = [](const glm::ivec2& p1, const glm::ivec2& p2) {
				return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
				};
			return distanceSquared(a->pos, pPos) < distanceSquared(b->pos, pPos);
			});
	}

	chunkLoadQueueMutex.unlock();
}

size_t World::chunksLoaded() {
	chunksMutex.lock();
	size_t size = chunks.size();
	chunksMutex.unlock();
	return size;
}

size_t World::chunkLoadQueueCount() {
	chunkLoadQueueMutex.lock();
	size_t size = chunkLoadQueue.size();
	chunkLoadQueueMutex.unlock();
	return size;
}

bool World::isChunkLoaded(glm::ivec2 cPos) {
	auto it = chunks.find(hashPos(cPos));
	if (it == chunks.end()) return false;
	return it->second->loaded;
}

Chunk* World::getChunk(glm::ivec2 cPos) {
	std::size_t chunkCh = hashPos(cPos);

	chunksMutex.lock();
	auto cit = chunks.find(chunkCh);
	chunksMutex.unlock();
	if (cit == chunks.end()) {
		return nullptr; // chunk not loaded
	}

	return cit->second;
}
