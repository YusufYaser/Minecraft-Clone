#include "World.h"
#include "Utils.h"
#include "../Game/Game.h"

void World::chunkLoaderFunc() {
	std::string name = Game::getInstance()->getLoadedWorldName();

	while (true) {
		if (unloading.load()) break;

		if (chunkLoadQueue.size() == 0) continue;
		chunkLoadQueueMutex.lock();
		glm::ivec2 pos = chunkLoadQueue.front();
		chunkLoadQueue.erase(chunkLoadQueue.begin());
		chunkLoadQueueMutex.unlock();

		std::size_t chunkCh = hashPos(pos);
		chunksMutex.lock();
		Chunk* chunk = chunks[chunkCh];
		chunksMutex.unlock();

		if (time(0) - chunk->lastRendered > 10) {
			delete chunk;
			chunks.erase(chunkCh);
			continue;
		}

		std::string path = "worlds/" + name + "/c" + std::to_string(chunkCh);
		if (std::filesystem::exists(path)) {
			std::ifstream chunkFile(path);
			ChunkSaveData* saveData = new ChunkSaveData();
			chunkFile.read(reinterpret_cast<char*>(saveData), sizeof(ChunkSaveData));
			chunkFile.close();

			for (int y = 0; y < MAX_HEIGHT; y++) {
				for (int x = 0; x < 16; x++) {
					for (int z = 0; z < 16; z++) {
						if (saveData->blocks[y][x][z] == BLOCK_TYPE::AIR) continue;

						glm::ivec3 bPos = {
							x + (pos.x * 16),
							y,
							z + (pos.y * 16),
						};

						setBlock(bPos, saveData->blocks[y][x][z]);
					}
				}
			}

			delete saveData;

			chunk->loaded = true;
			chunk->modified = false;
			continue;
		}

		if (generator == Generator::Void) {
			if (pos.x == 0 && pos.y == 0) {
				setBlock({ 0, 0, 0 }, BLOCK_TYPE::BEDROCK);
			}
			chunk->loaded = true;
			continue;
		}

		for (int x = pos.x * 16; x < 16 + pos.x * 16; x++) {
			for (int z = pos.y * 16; z < 16 + pos.y * 16; z++) {
				if (unloading.load()) break;

				int height = getHeight({ x, z });
				for (int y = 0; y < height; y++) {
					if (unloading.load()) break;
					BLOCK_TYPE type = BLOCK_TYPE::STONE;
					if (y == 0) {
						type = BLOCK_TYPE::BEDROCK;
					} else if (height > getSeaLevel()) {
						if (y == height - 1) type = BLOCK_TYPE::GRASS;
						else if (y >= height - 3) type = BLOCK_TYPE::DIRT;
					} else {
						if (y == height - 1) type = BLOCK_TYPE::SAND;
						else type = BLOCK_TYPE::STONE;
					}

					setBlock(glm::ivec3(x, y, z), type);
				}
				if (height <= getSeaLevel()) {
					for (int y = height; y < getSeaLevel(); y++) {
						setBlock(glm::ivec3(x, y, z), BLOCK_TYPE::WATER);
					}
				}

				int MAX_STRUCTURE_HEIGHT = 0;

				std::vector<Structure*> structuresInChunk;

				for (Structure* structure : structures) {
					if (!structure->isInXZ({ x, z })) continue;

					int sHeight = structure->getHeight();
					sHeight -= structure->getPivot().y;
					sHeight += height;
					if (sHeight > MAX_STRUCTURE_HEIGHT) MAX_STRUCTURE_HEIGHT = sHeight;

					structuresInChunk.push_back(structure);
				}

				if (structuresInChunk.size() == 0) continue;

				for (int y = 0; y < height + MAX_STRUCTURE_HEIGHT; y++) {
					for (Structure* structure : structuresInChunk) {
						BLOCK_TYPE blockType = structure->getBlock({ x, y, z });
						if (blockType != BLOCK_TYPE::NONE) {
							setBlock(glm::ivec3(x, y, z), blockType);
							break;
						}
					}
				}
			}
		}

		chunk->loaded = true;
		chunk->modified = false;
	}
}

void World::chunkUnloaderFunc() {
	std::string name = Game::getInstance()->getLoadedWorldName();

	while (true) {
		if (unloading.load() && chunks.size() == 0) break;
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		time_t current = time(nullptr);
		if (!chunksMutex.try_lock()) continue;
	unloader_func_chunks_loop:
		for (auto& [ch, chunk] : chunks) {
			if (chunk == nullptr) {
				chunks.erase(ch);
				continue;
			}
			if ((chunk->loaded && !chunk->permanentlyLoaded && current - chunk->lastRendered > 10)
				|| unloading.load()) {
				if (chunk->modified) {
					try {
						ChunkSaveData* saveData = new ChunkSaveData();
						chunk->blocksMutex.lock();
						for (auto& [ch, block] : chunk->blocks) {
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

						std::ofstream chunkFile("worlds/" + name + "/c" + std::to_string(ch));
						chunkFile.write(reinterpret_cast<const char*>(saveData), sizeof(ChunkSaveData));
						chunk->modified = false;
						chunkFile.close();

						delete saveData;
					} catch (std::filesystem::filesystem_error e) {
						error("FAILED TO SAVE CHUNK:", e.what());
					}
				}

				delete chunks[ch];
				chunks.erase(ch);
				goto unloader_func_chunks_loop;
			}
		}
		chunksMutex.unlock();
	}
}

void World::loadChunk(glm::ivec2 pos, bool permanentlyLoaded) {
	std::size_t chunkCh = hashPos(pos);
	auto it = chunks.find(chunkCh);

	if (it == chunks.end()) {
		Chunk* chunk = new Chunk();
		chunk->blocks = std::unordered_map<std::size_t, Block*>();
		for (int i = 0; i < BLOCK_TYPE_COUNT; i++) {
			chunk->renderingGroups[(BLOCK_TYPE)i] = std::vector<Block*>();
		}
		chunk->permanentlyLoaded = permanentlyLoaded;
		chunk->lastRendered = time(nullptr);
		chunksMutex.lock();
		chunks[chunkCh] = chunk;
		chunksMutex.unlock();
	} else {
		it->second->lastRendered = time(nullptr);
		return; // already loaded
	}

	chunkLoadQueueMutex.lock();
	chunkLoadQueue.push_back(pos);
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
